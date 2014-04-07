#ifndef __MESSAGING_H__
#define __MESSAGING_H__

extern int sendMessageRect(const char *ipAddr, float distance, float angle, float tension);
extern int sendMessagePole(const char *ipAddr, float distance, float angle, float anglePerp);

#endif
