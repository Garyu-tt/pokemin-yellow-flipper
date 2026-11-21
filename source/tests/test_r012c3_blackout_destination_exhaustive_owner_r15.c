#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "../yellow_blackout_destination.c"
int main(void) {
    for(unsigned map=0;map<256;map++)
      for(unsigned anchor=0;anchor<2;anchor++)
        for(unsigned faint=0;faint<2;faint++)
          for(unsigned corrupt=0;corrupt<2;corrupt++) {
            uint8_t out=0xa5u;
            bool expected=(map==1u && anchor==1u && faint==1u && corrupt==0u);
            bool ok=yellow_resolve_blackout_destination((uint8_t)map,anchor!=0,faint!=0,corrupt!=0,&out);
            assert(ok==expected);
            assert(out==(expected?YMapViridian:0xa5u));
            YellowBlackoutSpawn spawn={0xa5u,0xb6u,0xc7u};
            bool spawn_ok=yellow_resolve_blackout_spawn((uint8_t)map,anchor!=0,faint!=0,corrupt!=0,&spawn);
            assert(spawn_ok==expected);
            assert(spawn.native_map==(expected?YMapViridian:0xa5u));
            assert(spawn.x==(expected?23u:0xb6u));
            assert(spawn.y==(expected?26u:0xc7u));
          }
    assert(!yellow_resolve_blackout_destination(1,true,true,false,0));
    assert(!yellow_resolve_blackout_spawn(1,true,true,false,0));
    return 0;
}
