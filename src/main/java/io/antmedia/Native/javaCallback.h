#ifndef JAVA_CALLBACK_H
#define JAVA_CALLBACK_H

// Function pointer type for the callback
typedef void (*Callback)( char* streamId, char*  roomId , char* data );

// Callback registration function
void registerCallback(Callback cb);


#endif
