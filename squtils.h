SQRESULT sq_formatthrowerror(HSQAPI sq, HSQUIRRELVM v, const SQChar *s, ...)
{
	SQUnsignedInteger buf_size = 256 * sizeof(SQChar);
	SQChar* buf = sq->getscratchpad(v, buf_size);

	va_list vl;
	va_start(vl, s);
	scvsprintf(buf, buf_size, s, vl);
	va_end(vl);

	return sq->throwerror(v, buf);
}
