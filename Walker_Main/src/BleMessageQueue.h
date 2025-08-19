#ifndef BLEMESSAGEQUEUE_H
#define BLEMESSAGEQUEUE_H

#include "BleMessage.h"


namespace ble_queue
{


BleMessage pop();

void push(BleMessage* msg);

int size();


int check_for(BleMessage msg);


void clear();

};

#endif