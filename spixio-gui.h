#ifndef SPIXIO_GUI_H
#define SPIXIO_GUI_H

int get_ndig(int word, int radix);
int real_radix(char* buf2, char* chbuf2, int ndig, int bradix, int radix);
int change_radix(int bword, int word, int bradix, int radix);
int refill(int inputs, int outputs, int word, int radix);
void sprint_radix(char* chbuf2, uint32_t nvalue, int ndig, int radix);

#endif
