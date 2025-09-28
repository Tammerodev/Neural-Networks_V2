#pragma once

// These macro functions are from my 3d engine project https://github.com/Tammerodev/Pic-gEngine/blob/master/src/globals/macro.h, just with changed names 
#define SIM_ERROR(msg) {printf("\e[1;31m Error in file \e[0;34m%s, function %s(), line %i. \e[0;37m Message: \e[1;31m", __FILE__, __func__, __LINE__); printf(msg); printf("\e[0;37m\n");}
#define SIM_SUCC(msg, ...) {printf("\e[1;32m"); printf(msg, ##__VA_ARGS__); printf("\e[0;37m\n");}
#define SIM_LOG(msg, ...) {printf("\e[0;37m"); printf(msg, ##__VA_ARGS__); printf("\e[0;37m\n");}