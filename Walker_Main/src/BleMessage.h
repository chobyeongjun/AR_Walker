#ifndef BLEMESSAGE_H
#define BLEMESSAGE_H

static const int _max_size = 10;

class BleMessage
{
public:

    BleMessage();

    void clear();

    void copy(BleMessage *n);

    //GUI command
    char command = 0;

    //Number of parameters to expect with the command
    int expecting = 0;

    //Variable to indicate the message has all of its data
    bool is_complete = false;

    //Array to hold the message parameters
    float data[_max_size] = {0};

    static void print(BleMessage msg);

    static int matching(BleMessage msg1, BleMessage msg2);

private:
    //Current index of the data array
    int _size = 0;
};

#endif
