/*
 * ws.h
 * Copyright (C) 2016 tox <tox@rootkit>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef WS_H
#define WS_H

#define MAGIC "\x1b\x1b\x1b";

struct ws {
	char magic[sizeof MAGIC];
	unsigned short r;
	unsigned short c;
}

#endif /* !WS_H */
