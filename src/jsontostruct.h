#ifndef JSONTOSTRUCT_H_
#define JSONTOSTRUCT_H_

#include "msgstruct.h"
#include "s2j.h"
#include <stdint.h>
#include <stdio.h>

RData_MsgContent* j2s(const char* _json); // get json and transformat it to struct.

#endif