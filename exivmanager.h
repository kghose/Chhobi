#ifndef EXIVMANAGER_H
#define EXIVMANAGER_H

/*
 * Set of functions to handle the loading and saving of
 * photo metadata
 */
#include <QString>
#include "photo.h"

PhotoMetaData load_metadata(QString absolute_filename);
void save_metadata(PhotoMetaData pmd);

#endif // EXIVMANAGER_H
