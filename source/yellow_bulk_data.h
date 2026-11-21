#ifndef YELLOW_BULK_DATA_H
#define YELLOW_BULK_DATA_H
#include <stdbool.h>
#include "yellow_core.h"
#include "pydp_reader.h"
/* Source-derived Route1+ViridianForest encounter arrays. Return false if
   even one slot cannot be represented by native runtime species IDs. */
bool yellow_bulk_prepare(PydpPack* pack,YellowBulkWild* result);
bool yellow_bulk_prepare_lass(PydpPack* pack,YellowBulkLass* result);
#endif
