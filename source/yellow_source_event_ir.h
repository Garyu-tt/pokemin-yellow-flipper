#ifndef YELLOW_SOURCE_EVENT_IR_H
#define YELLOW_SOURCE_EVENT_IR_H
#include <stdint.h>
#include <stdbool.h>
enum {YEIR_WARP=1,YEIR_BG=2,YEIR_NPC=3,YEIR_TRAINER=4,YEIR_ITEM=5,
      YEIR_TEXT_POINTER=6,YEIR_SCRIPT_POINTER=7,YEIR_SAFE_OPCODE=8};
enum {YEIR_OP_TEXTBOX=1,YEIR_OP_JUMP_TEXTBOX=2,YEIR_OP_RETURN=3};
typedef struct {
 uint8_t map,x,y,kind,index,value,extra;
} YellowEventIR;
/* Read-only source data; not an executable general GameBoy ASM VM. */
uint16_t yellow_ir_count(void);
bool yellow_ir_at(uint16_t index,YellowEventIR* out);
bool yellow_ir_at_position(uint8_t map,int x,int y,uint8_t kind,YellowEventIR* out);
uint8_t yellow_ir_forest_item_mask(int x,int y);
bool yellow_ir_opcode_supported(uint8_t opcode);
#endif
