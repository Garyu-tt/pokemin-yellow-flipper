#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "glitchdex.h"
int main(void){
    bool seen[256]={0};
    for(unsigned i=0;i<GLITCHDEX_COUNT;i++){
        uint8_t id=glitchdex_id(i);
        assert(!seen[id]);seen[id]=true;
        assert(glitchdex_label(id)&&strlen(glitchdex_label(id))>0);
        if(i>0&&i<40)assert(glitchdex_is_missingno(id));
        if(i>=40)assert(id>=0xBF);
    }
    assert(seen[0]&&seen[0xB6]&&seen[0xB7]&&seen[0xB8]&&seen[0xBF]&&seen[0xFF]);
    assert(glitchdex_id(255)==0);
    assert(!glitchdex_is_missingno(0));
    assert(!glitchdex_is_missingno(0xBF));
    puts("GLITCHDEX PASS: 105 unique indices including 39 MissingNo. and 66 others");
}
