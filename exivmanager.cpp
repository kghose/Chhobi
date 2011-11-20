#include "exivmanager.h"
#include <exiv2/easyaccess.hpp> //for lensName and other convenience functions
#include <cassert> //Needed for assert
#include <sys/stat.h> //for the birthdate (needed for files w/o metadata)

const QStringList sidecar_types = (QStringList("avi") << "mov");

//TODO ifdef for mac
QDateTime mac_os_create_datetime(QString absolute_file_path)
{
    struct stat64 the_stat;
    stat64(absolute_file_path.toStdString().c_str(), &the_stat);
    return QDateTime::fromTime_t(the_stat.st_birthtimespec.tv_sec);
}

PhotoMetaData load_metadata(QString absolute_filename)
{
    PhotoMetaData pmd;
    pmd.absolute_file_path = absolute_filename;
    pmd.valid = true;
    QFileInfo qfi(absolute_filename);

    if(!qfi.exists()) {
        exiv_bad_metadata(pmd);
        return pmd;
    }

    //TODO more atomic error handling so we just ignore blank fields
    Exiv2::Image::AutoPtr image;

    //we use this when we have converted non-standard fields (like that WPG uses)
    //to IPTC standard - we need to resave it
    bool resave = false;

    if(sidecar_types.contains(qfi.suffix(), Qt::CaseInsensitive)) {//exiv2 can't read/write metadata
        load_sidecar(absolute_filename, pmd, resave);
    } else {//exiv2 can r/w metadata
        try {
            image =
            Exiv2::ImageFactory::open(absolute_filename.toStdString());
            assert (image.get() != 0);
            image->readMetadata();
        } catch (Exiv2::AnyError& e) {
            qDebug() << "Caught Exiv2 exception '" << QString(e.what()) << "'";
            exiv_bad_metadata(pmd);
            return pmd;
        }

        image->readMetadata();
        ipct_load_caption_and_keywords(image, pmd, resave);
        exiv_load_rotation(image, pmd, resave);
        exiv_load_date(image, pmd, resave);
        exiv_load_misc_ro(image, pmd, resave);
    }

    if(resave) {
        save_metadata(pmd);
        std::cout << "Saved WPG metadata for " << absolute_filename.toStdString() << std::endl;
    }

    //TODO error handling
    return pmd;
}

inline void exiv_bad_metadata(PhotoMetaData &pmd)
{
    pmd.valid = false;
    pmd.rotation_angle = 0;
}

//Convenience functions (to break up an otherwise overlong load_metadata)
inline void ipct_load_caption_and_keywords(Exiv2::Image::AutoPtr &image, PhotoMetaData &pmd, bool &resave)
{
    Exiv2::IptcData iptcData = image->iptcData();
    Exiv2::IptcKey exiv2_caption_iptc_key("Iptc.Application2.Caption");
    Exiv2::IptcData::iterator capt = iptcData.findKey(exiv2_caption_iptc_key);
    if(capt != iptcData.end()) {//end is the "not found" value
        //toString returns std::string, this needs to be convered by c_str
        //before QString will play ball
        pmd.caption = capt->toString().c_str();
    } else {
        //I had a lot of stuff in windows photo gallery that I'd like to
        //transfer. If there is no regular caption, we check in case there was
        //a windows photo gallery caption
        Exiv2::XmpData xmpData = image->xmpData();
        Exiv2::XmpKey xmpKey_title("Xmp.dc.title");
        Exiv2::XmpData::iterator xdi = xmpData.findKey(xmpKey_title);
        if(xdi != xmpData.end()) {
            pmd.caption = xdi->toString().c_str();
            pmd.caption.remove("lang=\"x-default\" ");//something funny XP puts in
            resave = true;//make sure we save the changes
        }
    }

    //read in all the keywords
    Exiv2::IptcData::iterator end = iptcData.end();
    QString keyword_iptc_key("Iptc.Application2.Keywords");
    for (Exiv2::IptcData::iterator md = iptcData.begin(); md != end; ++md) {
        if( md->key().c_str() == keyword_iptc_key )
            pmd.keywords << md->toString().c_str();
    }
    if(pmd.keywords.size() == 0) {
        //Check if there are any keywords from WPG
        Exiv2::XmpData xmpData = image->xmpData();
        Exiv2::XmpData::iterator xdi;
        QString key_subject("Xmp.dc.subject");
        for(xdi = xmpData.begin(); xdi != xmpData.end(); ++xdi) {
            if(xdi->key().c_str() == key_subject) {
                pmd.keywords << xdi->toString().c_str();
                resave = true;//make sure we save the changes
            }
        }
    }

}

inline void exiv_load_rotation(Exiv2::Image::AutoPtr &image, PhotoMetaData &pmd, bool &resave)
{
    Exiv2::ExifData exifData = image->exifData();
    //figure out rotation from EXIF data
    //http://sylvana.net/jpegcrop/exif_orientation.html
    switch(exifData["Exif.Image.Orientation"].toLong()) {
    case 1:
        pmd.rotation_angle = 0;
        break;
    case 3:
        pmd.rotation_angle = 180;
        break;
    case 6:
        pmd.rotation_angle = 90;
        break;
    case 8:
        pmd.rotation_angle = 270;
        break;
    case 2:
    case 4:
    case 5:
    case 7:
        std::cout << pmd.absolute_file_path.toStdString() << ": Rotation code: " <<
        exifData["Exif.Image.Orientation"].toLong() << " (" <<
        exifData["Exif.Image.Orientation"] << ") "
        "currently not handled by Chhobi." << std::endl;
        break;
    default:
        std::cout << pmd.absolute_file_path.toStdString() << ": Unknown rotation code: " <<
        exifData["Exif.Image.Orientation"].toLong() << ", setting to 1" << std::endl;
        pmd.rotation_angle = 0;
        resave = true;
    }
}

inline void exiv_load_date(Exiv2::Image::AutoPtr &image, PhotoMetaData &pmd, bool &resave)
{
    Exiv2::ExifData exifData = image->exifData();
    //grab image take date
    QString rawDate = exifData["Exif.Photo.DateTimeOriginal"].toString().c_str();
    qDebug() << rawDate;
    QString day_format = "yyyy:MM:dd",
            time_format = "hh:mm:ss";
    pmd.photo_date = QDateTime(QDate::fromString(rawDate.left(10), day_format),
            QTime::fromString(rawDate.right(8), time_format));

    //EXIF data does not store the time zone.
    //http://en.wikipedia.org/wiki/Exchangeable_image_file_format#Problems
    //When we store the date/time in the database we use .toTime_t which
    //converts the time to UTC based on the current time zone. This leads to
    //certain issues: suppose the picture was taken, downloaded from the
    //camera and then imported into Chhobi in a +0400 timezone. During the
    //import the time is read from the metadata as local time and stored in
    //the database as UTC. Suppose this photo is now edited in a -0400 time
    //zone but the time is left unaltered. The time in the
    //picture metadata is fine, but in the database the time is now
    //interpreted as the new local time and the UTC value is correspondingly
    //shifted. To avoid this problem, we change the time spec to UTC as we
    //load the date. This merely causes an ordering problem for pictures taken
    //during a given day in a different time-zone. Editing one of the pictures
    //would cause a shift in its time in the db, causing the pictures to be
    //located out of order in the db
    pmd.photo_date.setTimeSpec(Qt::UTC);
}

inline void exiv_load_misc_ro(Exiv2::Image::AutoPtr &image, PhotoMetaData &pmd, bool &resave)
{
    Exiv2::ExifData exifData = image->exifData();

    //Read only metadata - experimental - like exposure etc.
    Exiv2::Rational exposure_time_rat = exifData["Exif.Photo.ExposureTime"].toRational();
    pmd.exposure_time.numerator = exposure_time_rat.first;
    pmd.exposure_time.denominator = exposure_time_rat.second;

    Exiv2::Rational fnumber_rat = exifData["Exif.Photo.FNumber"].toRational();
    pmd.fnumber.numerator = fnumber_rat.first;
    pmd.fnumber.denominator = fnumber_rat.second;

    pmd.iso = exifData["Exif.Photo.ISOSpeedRatings"].toLong();

    Exiv2::Rational focal_len_rat = exifData["Exif.Photo.FocalLength"].toRational();
    pmd.focal_length.numerator = focal_len_rat.first;
    pmd.focal_length.denominator = focal_len_rat.second;

    pmd.camera_model = QString(exifData["Exif.Image.Model"].toString().c_str());
    if(Exiv2::lensName(exifData) != exifData.end())
        pmd.lens_model = Exiv2::lensName(exifData)->print(&exifData).c_str();
}

//For movie and other files that don't have metadata on file
inline void load_sidecar(QString absolute_filename, PhotoMetaData &pmd, bool &resave)
{
    QFile file(absolute_filename + ".meta");
    QHash<QString,QVariant> info;
    if(file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> info;
    } else {
        QFileInfo qfi(absolute_filename);
        info["date"] = QVariant(mac_os_create_datetime(absolute_filename));
        info["filename"] = QVariant(absolute_filename);
        info["keywords"] = QVariant(QStringList());
        resave = true;
    }

    pmd.absolute_file_path = info["filename"].toString();
    pmd.photo_date = info["date"].toDateTime();
    pmd.rotation_angle = 0;
    pmd.keywords = info["keywords"].toStringList();
    qDebug() << info;
}

void save_metadata(PhotoMetaData pmd)
{
    QFileInfo qfi(pmd.absolute_file_path);
    if(sidecar_types.contains(qfi.suffix(), Qt::CaseInsensitive)) {//exiv2 can't read/write metadata
        save_sidecar(qfi.absoluteFilePath(), pmd);
        return;
    }

    //TODO error handling
    Exiv2::Image::AutoPtr image =
    Exiv2::ImageFactory::open(pmd.absolute_file_path.toStdString());
    //assert (image.get() != 0);

    //read existing metadata
    image->readMetadata();

    //Overwrite IPTC caption and keywords
    Exiv2::IptcData &iptcData = image->iptcData();
    iptcData["Iptc.Application2.Caption"] = pmd.caption.toStdString();//TODO unicode handling?

    //The only way I know - erase all keywords and add em on fresh
    QString keyword_iptc_key("Iptc.Application2.Keywords");
    Exiv2::IptcKey exiv2_keyword_iptc_key("Iptc.Application2.Keywords");
    Exiv2::IptcData::iterator md = iptcData.findKey(exiv2_keyword_iptc_key);
    //we don't use a iterator loop because erase invalidates iterators and can
    //cause a 'pure virtual function called' error
    while(md != iptcData.end()) { //apparently this is the 'not found' value
        iptcData.erase(md);
        md = iptcData.findKey(exiv2_keyword_iptc_key);
    }

    //now write them afresh
    for(int n = 0 ; n < pmd.keywords.size() ; n++) {
        Exiv2::Iptcdatum exiv2_keyword(exiv2_keyword_iptc_key);
        exiv2_keyword.setValue(pmd.keywords.at(n).toStdString());
        iptcData.add(exiv2_keyword);
    }

    //Overwrite and save EXIV orientation data
    Exiv2::ExifData &exifData = image->exifData();
    short int rot_code;//remember ROTT?
    switch(pmd.rotation_angle) {
    case 0:
    case 360:
        rot_code = 1;
        break;
    case 90:
        rot_code = 6;
        break;
    case 180:
        rot_code = 3;
        break;
    case 270:
        rot_code = 8;
        break;
    }
    exifData["Exif.Image.Orientation"] = rot_code;

    //Overwrite the EXIV DateTimeOriginal
    QString format = "yyyy:MM:dd hh:mm:ss";
    exifData["Exif.Photo.DateTimeOriginal"]
             = pmd.photo_date.toString(format).toStdString();

    image->writeMetadata();

    //now we need to update the last modified time
    //QFileInfo fi(absolute_filename);
    //last_modified = fi.lastModified();

    //return true;
}

inline void save_sidecar(QString absolute_filename, PhotoMetaData &pmd)
{
    QFile file(absolute_filename + ".meta");
    QHash<QString,QVariant> info;
    info["date"] = QVariant(pmd.photo_date);
    info["filename"] = QVariant(pmd.absolute_file_path);
    info["keywords"] = QVariant(pmd.keywords);

    if(file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << info;
    } else {
        qDebug() << "Error saving metadata";
    }
}
