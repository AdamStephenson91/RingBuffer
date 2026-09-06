#pragma once
#include "Producer.h"
#include "Consumer.h"

// This number specifies the total number of bytes
// that will be pushed through the ring buffer
// (minus a small margin as messages won't fit perfectly)
Producer<100000> producer;
Consumer consumer;
