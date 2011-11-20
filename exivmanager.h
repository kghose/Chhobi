#ifndef EXIVMANAGER_H
#define EXIVMANAGER_H

/*
 * Set of functions to handle the loading and saving of
 * photo metadata
 */
#include <QString>
#include "photo.h"
#include <exiv2/image.hpp> //needed for exif operations
#include <exiv2/exif.hpp>

PhotoMetaData load_metadata(QString absolute_filename);
void save_metadata(PhotoMetaData pmd);

//Convenience functions (to break up an otherwise overlong load_metadata)
inline void exiv_bad_metadata(PhotoMetaData &);
inline void ipct_load_caption_and_keywords(Exiv2::Image::AutoPtr &, PhotoMetaData &, bool &);
inline void exiv_load_rotation(Exiv2::Image::AutoPtr &, PhotoMetaData &, bool &);
inline void exiv_load_date(Exiv2::Image::AutoPtr &, PhotoMetaData &, bool &);
inline void exiv_load_misc_ro(Exiv2::Image::AutoPtr &, PhotoMetaData &, bool &);

//For movie and other files that don't have metadata on file
inline void load_sidecar(QString, PhotoMetaData &, bool &);

inline void save_sidecar(QString, PhotoMetaData &);

#endif // EXIVMANAGER_H
