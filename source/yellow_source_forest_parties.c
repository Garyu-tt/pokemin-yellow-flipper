#include "yellow_source_forest_parties.h"
/* 5 original Viridian Forest trainer parties. Verified original revision
   and exact 1-based trainer class numbering. */
static const YSourceForestParty party_source[5]={
 {1,1,2,{{YK_CATERPIE,7},{YK_CATERPIE,7},{0,0}}}, /* BugCatcher #1 */
 {1,2,3,{{YK_METAPOD,6},{YK_CATERPIE,6},{YK_METAPOD,6}}}, /* BugCatcher #2 */
 {1,3,1,{{YK_CATERPIE,10},{0,0},{0,0}}}, /* BugCatcher #3 */
 {1,15,2,{{YK_CATERPIE,8},{YK_METAPOD,8},{0,0}}}, /* BugCatcher #15 */
 {2,19,2,{{YK_NIDORAN_F,6},{YK_NIDORAN_M,6},{0,0}}}, /* Lass #19 */
};
const YSourceForestParty* y_source_forest_party(uint8_t trainer) {
 return trainer<5?&party_source[trainer]:0;
}
bool y_source_forest_mon(uint8_t trainer,uint8_t member,uint8_t* species,uint8_t* level) {
 const YSourceForestParty* t=y_source_forest_party(trainer);
 if(!t||!species||!level||member>=t->count)return false;
 *species=t->mon[member].species;
 *level=t->mon[member].level;
 return true;
}
