#ifndef FILEMAN_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)7
#define FILEMAN_H
extern int Load_Config(int fdc, struct Config *cfg);
extern int Set_Config_Default(int fdc, struct Config *cfg);
#endif
