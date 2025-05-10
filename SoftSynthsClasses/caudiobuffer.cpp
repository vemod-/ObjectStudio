#include "caudiobuffer.h"

CAudioBuffer::~CAudioBuffer() { deleteData(); }

CMonoBuffer::~CMonoBuffer(){}

CStereoBuffer::~CStereoBuffer(){
    delete leftBuffer;
    delete rightBuffer;
}
