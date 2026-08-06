#ifndef __PARAM_MARSHAL_H__
#define __PARAM_MARSHAL_H__
// parameterValue[] registry and its CHAN_ARV marshalling, extracted from
// bsp_json.cpp/handleData.cpp for host-side testing. Arduino-free.
#include "bsp_json.h"
#include "kdu_protocol.h"

extern ParameterValue_t parameterValue[ITEMSUM];

void readChanFromArray(CHAN_ARV_P B);
void writeChanToArray(CHAN_ARV_P B);
#endif
