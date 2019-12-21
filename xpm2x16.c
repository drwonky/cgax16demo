#include <stdio.h>

/*
".  c #000000",
"+  c #0000A8",
"@  c #00A800",
"#  c #00A8A8",
"$  c #A80000",
"%  c #A800A8",
"&  c #A85700",
"*  c #A8A8A8",
"=  c #575757",
"-  c #5757FF",
";  c #57FF57",
">  c #57FFFF",
",  c #FF5757",
"'  c #FF57FF",
")  c #FFFF57",
"!  c #FFFFFF"
*/

char map(int c)
{
	char *m = ".+@#$%&*=-;>,')!";
	char r;

	for (r=0;r<16;r++) if (m[r]==(unsigned char) c) return r;

	return -1;

}

int main(void)
{
	char d,flag=0;
	unsigned char attr=0;
	int c,cnt=0;

	fprintf(stdout,"unsigned char xpm[] = {\n");

	while(!feof(stdin)) {
		c = fgetc(stdin);

		if (c == EOF) {
			fprintf(stdout,"0x%02x",attr);
			break;
		} else if (cnt&1) { 
			fprintf(stdout,"0x%02x,",attr);
		}

		if (c == '\r' || c == '\n') {
			fputc(c,stdout);
			cnt=0;
			continue;
		}

		d=map(c);

		if (d == -1) {
			fputc(c,stdout);
			continue;
		}

		if (cnt&1) {
			attr<<=4;
		}

		attr |= d;

		cnt++;
	}

	fprintf(stdout,"};");

	return 0;

}
