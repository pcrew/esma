
#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#define IS_DIGIT(p) \
        (*p >= '0' && *p <= '9')

#define IS_NOT_DIGIT(p) \
        !(IS_DIGIT(p))

#define IS_LETTER(p) \
        (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')

#define IS_NOT_LETTER(p) \
        !(IS_CHAR(p))

#define IS_LF(p) \
        *p == '\n'

#define IS_NOT_LF(p) \
        !(IS_LF(p))

#define IS_QUOTE(p) \
	*p == '"'

#define IS_NOT_QUOTE(p) \
	!(IS_QUOTE(p))

#define IS_UNDERSCORE(p) \
	*p == '_'

#define IS_NOT_UNDERSCORE(p) \
	!(IS_UNDERSCORE(p))

#define IS_COLON(p) \
        *p == ':'

#define IS_NOT_COLON(p) \
        !(IS_COLON(p))

#define IS_SEMICOLON(p) \
        *p == ';'

#define IS_NOT_SEMICOLON(p) \
        !(IS_SEMICOLON(p))

#define IS_OPEN_BRACE(p) \
        *p == '{'

#define IS_NOT_OPEN_BRACE(p) \
        !(IS_OPEN_BRACE(p))

#define IS_CLOSE_BRACE(p) \
        *p == '}'

#define IS_NOT_CLOSE_BRACE(p) \
        !(IS_CLOSE_BRACE(p))

#define IS_NOT_DELIM(p) \
        !(IS_DELIM(p))

#define str_3_cmp(p, c1, c2, c3)		\
	(*p	 == c1 &&			\
	 *(p + 1) == c2 &&			\
	 *(p + 2) == c3)

#define str_4_cmp(p, c1, c2, c3, c4) 		\
        (*p       == c1 &&               	\
         *(p + 1) == c2 &&               	\
         *(p + 2) == c3 &&               	\
         *(p + 3) == c4)

#define str_5_cmp(p, c1, c2, c3, c4, c5)	\
        (*p       == c1 &&               	\
         *(p + 1) == c2 &&               	\
         *(p + 2) == c3 &&               	\
         *(p + 3) == c4 &&               	\
         *(p + 4) == c5)

#define str_6_cmp(p, c1, c2, c3, c4, c5, c6)	\
	(*p       == c1 &&               	\
         *(p + 1) == c2 &&               	\
         *(p + 2) == c3 &&               	\
         *(p + 3) == c4 &&               	\
         *(p + 4) == c5 &&               	\
         *(p + 5) == c6)

#define str_7_cmp(p, c1, c2, c3, c4, c5, c6, c7)\
	(*p       == c1 &&               	\
         *(p + 1) == c2 &&               	\
         *(p + 2) == c3 &&               	\
         *(p + 3) == c4 &&               	\
         *(p + 4) == c5 &&               	\
         *(p + 5) == c6 &&			\
	 *(p + 6) == c7)

#define str_8_cmp(p, c1, c2, c3, c4, c5, c6, c7, c8)	\
	(*p       == c1 &&               		\
         *(p + 1) == c2 &&               		\
         *(p + 2) == c3 &&               		\
         *(p + 3) == c4 &&               		\
         *(p + 4) == c5 &&               		\
         *(p + 5) == c6 &&				\
	 *(p + 6) == c7 &&				\
	 *(p + 7) == c8)

#endif
