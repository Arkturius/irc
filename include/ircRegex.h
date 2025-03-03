#pragma once

# define	R_CHAR_RANGE(s, e)	s "-" e
# define	R_CHAR_GROUP(g)		"[" g "]"
# define	R_CHAR_INV_GROUP(g)	"[^" g "]"
# define	R_CAPTURE(c)		"(" c ")"

# define	R_1_OR_MORE(t)		t "+"
# define	R_0_OR_MORE(t)		t "*"
# define	R_0_OR_1(t)			t "?"
# define	R_AS_FEW(t)			R_0_OR_1(t)

# define	R_X_OR_MORE(t, x)	t "{"#x",}"
# define	R_X_EXACT(t, x)		t "{"#x"}"
# define	R_X_TO_Y(t, x, y)	t "{"#x","#y"}"
# define	R_1_TO_Y(t, y)		R_X_TO_Y(t,1,y)
# define	R_ALTERNATION(x, y)	x "|" y

# define	R_DIGIT_RANGE		R_CHAR_RANGE("0","9")
# define	R_LOWER_RANGE		R_CHAR_RANGE("a","z")
# define	R_UPPER_RANGE		R_CHAR_RANGE("A","Z")
# define	R_ALPHA_RANGE		R_LOWER_RANGE R_UPPER_RANGE
# define	R_ALNUM_RANGE		R_DIGIT_RANGE R_ALPHA_RANGE

# define	R_DIGIT				R_CHAR_GROUP(R_DIGIT_RANGE)
# define	R_LOWER				R_CHAR_GROUP(R_LOWER_RANGE)
# define	R_UPPER				R_CHAR_GROUP(R_UPPER_RANGE)
# define	R_ALPHA				R_CHAR_GROUP(R_ALPHA_RANGE)
# define	R_ALNUM				R_CHAR_GROUP(R_ALNUM_RANGE)

# define	R_ANY				"."
# define	R_START_STRING		"^"
# define	R_END_STRING		"$"
# define	R_FULL_MATCH(s)		R_START_STRING s R_END_STRING
# define	R_ESCAPE(c)			"\\" c
# define	R_BACKREF(n)		"\\"#n

# define	R_CAPTURE_INT		R_CAPTURE(R_1_OR_MORE(R_DIGIT))
# define	R_CAPTURE_WORD		R_CAPTURE(R_1_OR_MORE(R_ALPHA))

# define	R_SPACE				" "
# define	R_NOCRLF			R_CHAR_INV_GROUP("\r\n")
# define	R_NOSPCRLFCL		R_CHAR_INV_GROUP("\r\n :")


# define	IRC_NOAUTH_COMMANDS		"PASS|NICK|USER|PONG|QUIT"
# define	IRC_VALID_COMMANDS		IRC_NOAUTH_COMMANDS "|PING|JOIN|KICK|PRIVMSG|TOPIC|PART|MODE"

# define	R_IRC_NOAUTH_COMMANDS	R_FULL_MATCH(R_CAPTURE(IRC_NOAUTH_COMMANDS))
# define	R_IRC_VALID_COMMANDS	R_FULL_MATCH(R_CAPTURE(IRC_VALID_COMMANDS))


# define	R_MIDDLE			R_NOSPCRLFCL R_0_OR_MORE(R_CAPTURE(":|" R_NOSPCRLFCL))
# define	R_MIDDLE_PARAM		R_SPACE R_0_OR_1(R_CAPTURE(R_MIDDLE))

# define	R_TRAILING			R_0_OR_MORE(R_NOCRLF)
# define	R_TRAILING_PARAM	R_SPACE	R_0_OR_1(R_CAPTURE(R_TRAILING))


# define	NICKNAME_CHAR		" ,\\*\\?!@#"
# define	NICKNAME_START		NICKNAME_CHAR ":$"

# define	R_NICKNAME			R_FULL_MATCH											\
							(														\
								R_CAPTURE											\
								(													\
									R_CHAR_INV_GROUP(NICKNAME_START)				\
									R_X_TO_Y(R_CHAR_INV_GROUP(NICKNAME_CHAR),0,8)	\
								)													\
							)

# define	R_USERNAME	R_FULL_MATCH										\
					(													\
						R_CAPTURE										\
						(												\
							R_1_OR_MORE(R_CHAR_INV_GROUP("\r\n @"))		\
						)												\
					)


# define	R_COMMAND_MNEMO			R_CAPTURE_WORD


# define	R_CHANNEL_PREFIX	R_CHAR_GROUP("&#+")
# define	R_CHANNEL_ID		"!" R_X_EXACT(R_CHAR_GROUP(R_DIGIT_RANGE R_UPPER_RANGE),5)
# define	R_CHANNEL_CHAR		R_CHAR_INV_GROUP(" \x07,")

# define	R_CHANNEL_NAME		R_ALTERNATION											\
								(														\
									R_CHANNEL_PREFIX	R_1_TO_Y(R_CHANNEL_CHAR, 49),	\
									R_CHANNEL_ID		R_1_TO_Y(R_CHANNEL_CHAR, 45)	\
								)

# define	R_CHANNEL_KEY_CHAR	R_CHAR_INV_GROUP(" \x09\x10\x11\x12\x13,")

# define	R_CHANNEL_KEY		R_1_TO_Y(R_CHANNEL_KEY_CHAR, 23)

# define	R_CAPTURE_CHANNEL_NAME	R_CAPTURE(R_CHANNEL_NAME)
# define	R_CAPTURE_CHANNEL_KEY	R_CAPTURE(R_CHANNEL_KEY)

# define	R_CAPTURE_TARGET_NAME R_CAPTURE(R_ALTERNATION(R_CHANNEL_NAME, R_CHAR_INV_GROUP(NICKNAME_START)R_X_TO_Y(R_CHAR_INV_GROUP(NICKNAME_CHAR),0,8)))
