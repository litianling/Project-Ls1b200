
#include <string.h>

/*
 * char *strstr(p,q) returns a ptr to q in p, else 0 if not found
 */

char *strstr(const char *p, const char *q)
{
	const char *s, *t;

	if (!p || !q)
		return (0);

	if (!*q)
		return ((char *)p);
		
	for (; *p; p++)
    {
		if (*p == *q)
        {
			t = p;
			s = q;
			for (; *t; s++, t++)
            {
				if (*t != *s)
					break;
			}
			if (!*s)
				return ((char *)p);
		}
	}
	
	return (0);
}
