#include <maxminddb.h>
#include "squtils.h"

static SQRESULT from_entry_data_list(HSQUIRRELVM v, MMDB_entry_data_list_s **entry_data_list);

static SQRESULT from_map(HSQUIRRELVM v, MMDB_entry_data_list_s **entry_data_list)
{
	sq->newtable(v); // push new table

	const uint32_t map_size = (*entry_data_list)->entry_data.data_size;
	for (uint32_t i = 0; i < map_size && entry_data_list; ++i)
	{
		*entry_data_list = (*entry_data_list)->next;

		sq->pushstring(v, (*entry_data_list)->entry_data.utf8_string, (*entry_data_list)->entry_data.data_size); // key

		*entry_data_list = (*entry_data_list)->next;

		SQRESULT res = from_entry_data_list(v, entry_data_list); // push new object
		if (SQ_FAILED(res))
		{
			sq->pop(v, 2); // pop string and table
			return res;
		}

		sq->newslot(v, -3, SQFalse); // pop key and value
	}
	return 1; // return table
}

static SQRESULT from_array(HSQUIRRELVM v, MMDB_entry_data_list_s **entry_data_list)
{
	sq->newarray(v, 0); // push new array

	const uint32_t size = (*entry_data_list)->entry_data.data_size;
	for (uint32_t i = 0; i < size && entry_data_list; ++i)
	{
		*entry_data_list = (*entry_data_list)->next;

		SQRESULT res = from_entry_data_list(v, entry_data_list); // push new object
		if (SQ_FAILED(res))
		{
			sq->pop(v, 1); // pop array
			return res;
		}
		sq->arrayappend(v, -2); // pop value and push to array
	}
	return 1; // return array
}

static SQRESULT from_entry_data_list(HSQUIRRELVM v, MMDB_entry_data_list_s **entry_data_list)
{
	if (entry_data_list == NULL || *entry_data_list == NULL)
	{
		return sq->throwerror(v, _SC("Error while looking up data. Your database may be corrupt or you have found a bug in libmaxminddb."));
	}

	switch ((*entry_data_list)->entry_data.type)
	{
	case MMDB_DATA_TYPE_MAP:
		return from_map(v, entry_data_list);
	case MMDB_DATA_TYPE_ARRAY:
		return from_array(v, entry_data_list);
	case MMDB_DATA_TYPE_UTF8_STRING:
		sq->pushstring(v, (*entry_data_list)->entry_data.utf8_string, (*entry_data_list)->entry_data.data_size);
		return 1;
	case MMDB_DATA_TYPE_BYTES: // FIXME
		output_console_message("warning: MMDB_DATA_TYPE_BYTES is not implemented.");
		sq->pushnull(v);
		return 1;
		/*return PyByteArray_FromStringAndSize(
			(const char *)(*entry_data_list)->entry_data.bytes,
			(Py_ssize_t)(*entry_data_list)->entry_data.data_size);*/
	case MMDB_DATA_TYPE_DOUBLE:
		sq->pushfloat(v, (SQFloat)(*entry_data_list)->entry_data.double_value);
		return 1;
	case MMDB_DATA_TYPE_FLOAT:
		sq->pushfloat(v, (*entry_data_list)->entry_data.float_value);
		return 1;
	case MMDB_DATA_TYPE_UINT16:
		sq->pushinteger(v, (*entry_data_list)->entry_data.uint16);
		return 1;
	case MMDB_DATA_TYPE_UINT32:
		sq->pushinteger(v, (*entry_data_list)->entry_data.uint32);
		return 1;
	case MMDB_DATA_TYPE_BOOLEAN:
		sq->pushbool(v, (*entry_data_list)->entry_data.boolean);
		return 1;
	case MMDB_DATA_TYPE_UINT64: // FIXME
		output_console_message("warning: MMDB_DATA_TYPE_UINT64 is not implemented.");
		sq->pushnull(v);
		return 1;
		/*return PyLong_FromUnsignedLongLong(
			(*entry_data_list)->entry_data.uint64);*/
	case MMDB_DATA_TYPE_UINT128: // FIXME
		output_console_message("warning: MMDB_DATA_TYPE_UINT128 is not implemented.");
		sq->pushnull(v);
		return 1;
		//return from_uint128(*entry_data_list);
	case MMDB_DATA_TYPE_INT32:
		sq->pushinteger(v, (*entry_data_list)->entry_data.int32);
		return 1;
	}
	return sq_formatthrowerror(sq, v, _SC("Invalid data type arguments: %d"), (*entry_data_list)->entry_data.type);
}

#define SETUP_MMDB(v) \
    MMDB_s* mmdb = NULL; \
    sq->getinstanceup(v, 1, (SQUserPointer*)&mmdb, 0);

static SQInteger _mmdb_releasehook(SQUserPointer p, SQInteger SQ_UNUSED_ARG(size))
{
	MMDB_s* mmdb = (MMDB_s*)p;
	MMDB_close(mmdb);
	free(mmdb);
	return 1;
}

static SQInteger _mmdb_constructor(HSQUIRRELVM v)
{
	const SQChar* filename;
	sq->getstring(v, 2, &filename);

	MMDB_s* mmdb = (MMDB_s*)malloc(sizeof(MMDB_s));
	if (mmdb == NULL)
	{
		return sq->throwerror(v, MMDB_strerror(MMDB_OUT_OF_MEMORY_ERROR));
	}

	int status = MMDB_open(filename, MMDB_MODE_MMAP, mmdb);
	if (status != MMDB_SUCCESS)
	{
		free(mmdb);
		return sq_formatthrowerror(sq, v, _SC("Error opening database file (%s). %s"), filename, MMDB_strerror(status));
	}

	sq->setinstanceup(v, 1, mmdb);
	sq->setreleasehook(v, 1, _mmdb_releasehook);

	return 0;
}

static SQInteger _mmdb_get(HSQUIRRELVM v)
{
	SETUP_MMDB(v);

	const SQChar* ip_address;
	sq->getstring(v, 2, &ip_address);

	int gai_error = 0;
	int mmdb_error = MMDB_SUCCESS;
	MMDB_lookup_result_s result = MMDB_lookup_string(mmdb, ip_address, &gai_error, &mmdb_error);

	if (gai_error != 0)
	{
		return sq_formatthrowerror(sq, v, _SC("'%s' does not appear to be an IPv4 or IPv6 address."), ip_address);
	}

	if (mmdb_error != MMDB_SUCCESS)
	{
		return sq_formatthrowerror(sq, v, _SC("Error looking up %s. %s"), ip_address, MMDB_strerror(mmdb_error));
	}

	if (!result.found_entry)
	{
		return 0; // return null;
	}

	MMDB_entry_data_list_s* entry_data_list = NULL;
	int status = MMDB_get_entry_data_list(&result.entry, &entry_data_list);
	if (status != MMDB_SUCCESS)
	{
		SQRESULT res = sq_formatthrowerror(sq, v, _SC("Error while looking up data for %s. %s"), ip_address, MMDB_strerror(status));
		MMDB_free_entry_data_list(entry_data_list);
		return res;
	}

	MMDB_entry_data_list_s* original_entry_data_list = entry_data_list;
	SQRESULT res = from_entry_data_list(v, &entry_data_list);
	MMDB_free_entry_data_list(original_entry_data_list);
	return res;
}

static SQInteger _mmdb_metadata(HSQUIRRELVM v)
{
	SETUP_MMDB(v);

	MMDB_entry_data_list_s* entry_data_list;
	MMDB_get_metadata_as_entry_data_list(mmdb, &entry_data_list);
	MMDB_entry_data_list_s* original_entry_data_list = entry_data_list;

	SQRESULT res = from_entry_data_list(v, &entry_data_list);
	MMDB_free_entry_data_list(original_entry_data_list);

	return res;
}

static SQInteger _mmdb__typeof(HSQUIRRELVM v)
{
	sq->pushstring(v, _SC("MMDB"), -1);
	return 1;
}

#define _DECL_MMDB_FUNC(name, nparams, pmask) {_SC(#name), _mmdb_##name, nparams, pmask}
static const SQRegFunction mmdb_funcs[] = {
	_DECL_MMDB_FUNC(constructor, 2, _SC(".s")),
	_DECL_MMDB_FUNC(get, 2, _SC("xs")),
	_DECL_MMDB_FUNC(metadata, 1, _SC("x")),
	_DECL_MMDB_FUNC(_typeof, 1, _SC("x")),
	{NULL, (SQFUNCTION)0, 0, NULL}
};
#undef _DECL_MMDB_FUNC

static void register_sq_funcs(HSQUIRRELVM v)
{
	sq->pushroottable(v); // key for newslot
	sq->pushstring(v, _SC("MMDB"), -1);
	sq->newclass(v, SQFalse); // push new class
	for (SQInteger i = 0; mmdb_funcs[i].name != 0; ++i)
	{
		const SQRegFunction* f = &mmdb_funcs[i];
		sq->pushstring(v, f->name, -1); // key
		sq->newclosure(v, f->f, 0); // push new closure
		sq->setparamscheck(v, f->nparamscheck, f->typemask);
		sq->setnativeclosurename(v, -1, f->name);
		sq->newslot(v, -3, SQFalse); // pop key and value
	}
	sq->newslot(v, -3, SQFalse); // pop key and value
	sq->pop(v, 1); // pop root table
}
