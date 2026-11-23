#include "../dashkit_body_pure.h"
#include <assert.h>
#include <string.h>
using namespace dashkit_body;
int main(){
 uint8_t frame[8];
 assert(compose(OPEN_FRUNK,frame));assert(littleBits(frame,54,2)==2);assert(littleBits(frame,24,2)==0);
 for(uint8_t i=0;i<8;i++)if(i!=6)assert(frame[i]==0);
 assert(compose(OPEN_TRUNK,frame));assert(littleBits(frame,54,2)==1);
 assert(compose(FOLD_MIRRORS,frame));assert(littleBits(frame,24,2)==1);
 assert(compose(UNFOLD_MIRRORS,frame));assert(littleBits(frame,24,2)==2);
 assert(!compose(NONE,frame));assert(actionParse("mirror_fold")==FOLD_MIRRORS);
 assert(actionParse("mirror_fold_extra")==NONE);
 assert(actionParse("frunk")==OPEN_FRUNK);assert(actionParse("unknown")==NONE);
 uint8_t gear[8]={};putLittleBits(gear,21,3,1);assert(decodeGear(gear)==GEAR_P);
 putLittleBits(gear,21,3,4);assert(decodeGear(gear)==4);
 uint8_t speed[8]={};putLittleBits(speed,12,12,500&255); // only low byte write; test raw decode via hand-built bits
 speed[2]=(uint8_t)((500u>>4)&0xffu);speed[1]=(uint8_t)((500u&0xFu)<<4);
 assert(speedStopped(speed));speed[2]=(uint8_t)((510u>>4)&0xffu);speed[1]=(uint8_t)((510u&0xFu)<<4);assert(!speedStopped(speed));
 assert(permitted(true,true,true,true,GEAR_P,true,1000,800,820,900));
 assert(!permitted(false,true,true,true,GEAR_P,true,1000,800,820,900));
 assert(!permitted(true,false,true,true,GEAR_P,true,1000,800,820,900));
 assert(!permitted(true,true,true,true,4,true,1000,800,820,900));
 assert(!permitted(true,true,true,true,GEAR_P,false,1000,800,820,900));
 assert(!permitted(true,true,true,true,GEAR_P,true,1000,300,820,900));
 assert(!permitted(true,true,true,true,GEAR_P,true,1000,800,200,900));
 assert(!permitted(true,true,true,true,GEAR_P,true,1000,800,820,0));
 return 0;
}
