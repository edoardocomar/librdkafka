/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2012,2013 Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once


#include "rdendian.h"




/*
 * Kafka protocol definitions.
 */

#define RD_KAFKA_PORT      9092
#define RD_KAFKA_PORT_STR "9092"


/**
 * Request types
 */
struct rd_kafkap_reqhdr {
	int32_t  Size;
	int16_t  ApiKey;
#define RD_KAFKAP_None         -1
#define RD_KAFKAP_Produce       0
#define RD_KAFKAP_Fetch         1
#define RD_KAFKAP_Offset        2
#define RD_KAFKAP_Metadata      3
#define RD_KAFKAP_LeaderAndIsr  4
#define RD_KAFKAP_StopReplica   5
#define RD_KAFKAP_OffsetCommit  8
#define RD_KAFKAP_OffsetFetch   9
#define RD_KAFKAP_GroupCoordinator 10
#define RD_KAFKAP_JoinGroup     11
#define RD_KAFKAP_Heartbeat     12
#define RD_KAFKAP_LeaveGroup    13
#define RD_KAFKAP_SyncGroup     14
#define RD_KAFKAP_DescribeGroups 15
#define RD_KAFKAP_ListGroups    16
#define RD_KAFKAP_SaslHandshake 17
#define RD_KAFKAP_ApiVersion    18
#define RD_KAFKAP__NUM          19
	int16_t  ApiVersion;
	int32_t  CorrId;
	/* ClientId follows */
};

#define RD_KAFKAP_REQHDR_SIZE (4+2+2+4)
#define RD_KAFKAP_RESHDR_SIZE (4+4)

/**
 * Response header
 */
struct rd_kafkap_reshdr {
	int32_t  Size;
	int32_t  CorrId;
};



static RD_UNUSED
const char *rd_kafka_ApiKey2str (int16_t ApiKey) {
	static const char *names[] = {
		[RD_KAFKAP_Produce] = "Produce",
		[RD_KAFKAP_Fetch] = "Fetch",
		[RD_KAFKAP_Offset] = "Offset",
		[RD_KAFKAP_Metadata] = "Metadata",
		[RD_KAFKAP_LeaderAndIsr] = "LeaderAndIsr",
		[RD_KAFKAP_StopReplica] = "StopReplica",
		[RD_KAFKAP_OffsetCommit] = "OffsetCommit",
		[RD_KAFKAP_OffsetFetch] = "OffsetFetch",
		[RD_KAFKAP_GroupCoordinator] = "GroupCoordinator",
                [RD_KAFKAP_JoinGroup] = "JoinGroup",
                [RD_KAFKAP_Heartbeat] = "Heartbeat",
                [RD_KAFKAP_LeaveGroup] = "LeaveGroup",
                [RD_KAFKAP_SyncGroup] = "SyncGroup",
		[RD_KAFKAP_DescribeGroups] = "DescribeGroups",
		[RD_KAFKAP_ListGroups] = "ListGroups",
		[RD_KAFKAP_SaslHandshake] = "SaslHandshake",
		[RD_KAFKAP_ApiVersion] = "ApiVersion"

	};
	static RD_TLS char ret[32];

	if (ApiKey < 0 || ApiKey >= (int)RD_ARRAYSIZE(names)) {
		rd_snprintf(ret, sizeof(ret), "Unknown-%hd?", ApiKey);
		return ret;
	}

	return names[ApiKey];
}





/* Offset + MessageSize */
#define RD_KAFKAP_MESSAGESET_HDR_SIZE (8+4)
/* CRC + Magic + Attr + KeyLen + ValueLen + [Timestamp]
 * @remark This includes the optional (MsgVer=1) Timestamp field (8 bytes) */
#define RD_KAFKAP_MESSAGE_HDR_SIZE    (4+1+1+4+4+8)
/* Maximum per-message overhead. */
#define RD_KAFKAP_MESSAGE_OVERHEAD  \
	(RD_KAFKAP_MESSAGESET_HDR_SIZE + RD_KAFKAP_MESSAGE_HDR_SIZE)



/**
 * @brief ApiKey version support tuple.
 */
struct rd_kafka_ApiVersion {
	int16_t ApiKey;
	int16_t MinVer;
	int16_t MaxVer;
};

/**
 * @brief ApiVersion.ApiKey comparator.
 */
static RD_UNUSED int rd_kafka_ApiVersion_key_cmp (const void *_a, const void *_b) {
	const struct rd_kafka_ApiVersion *a = _a, *b = _b;

	return a->ApiKey - b->ApiKey;
}




/**
 *
 * Kafka protocol string representation prefixed with a convenience header
 *
 * Serialized format:
 *  { uint16, data.. }
 *
 */
typedef struct rd_kafkap_str_s {
	/* convenience header (aligned access, host endian) */
	int         len; /* Kafka string length (-1=NULL, 0=empty, >0=string) */
	const char *str; /* points into data[] or other memory,
			  * not NULL-terminated */
} rd_kafkap_str_t;


#define RD_KAFKAP_STR_LEN_NULL -1
#define RD_KAFKAP_STR_IS_NULL(kstr) ((kstr)->len == RD_KAFKAP_STR_LEN_NULL)

/* Returns the length of the string of a kafka protocol string representation */
#define RD_KAFKAP_STR_LEN0(len) ((len) == RD_KAFKAP_STR_LEN_NULL ? 0 : (len))
#define RD_KAFKAP_STR_LEN(kstr) RD_KAFKAP_STR_LEN0((kstr)->len)

/* Returns the actual size of a kafka protocol string representation. */
#define RD_KAFKAP_STR_SIZE0(len) (2 + RD_KAFKAP_STR_LEN0(len))
#define RD_KAFKAP_STR_SIZE(kstr) RD_KAFKAP_STR_SIZE0((kstr)->len)


/* Serialized Kafka string: only works for _new() kstrs */
#define RD_KAFKAP_STR_SER(kstr)  ((kstr)+1)

/* Macro suitable for "%.*s" printing. */
#define RD_KAFKAP_STR_PR(kstr)						\
	(int)((kstr)->len == RD_KAFKAP_STR_LEN_NULL ? 0 : (kstr)->len), \
		(kstr)->str

/* strndupa() a Kafka string */
#define RD_KAFKAP_STR_DUPA(destptr,kstr) \
	rd_strndupa((destptr), (kstr)->str, RD_KAFKAP_STR_LEN(kstr))

/* strndup() a Kafka string */
#define RD_KAFKAP_STR_DUP(kstr) rd_strndup((kstr)->str, RD_KAFKAP_STR_LEN(kstr))

/**
 * Frees a Kafka string previously allocated with `rd_kafkap_str_new()`
 */
static RD_UNUSED void rd_kafkap_str_destroy (rd_kafkap_str_t *kstr) {
	rd_free(kstr);
}



/**
 * Allocate a new Kafka string and make a copy of 'str'.
 * If 'len' is -1 the length will be calculated.
 * Supports Kafka NULL strings.
 * Nul-terminates the string, but the trailing \0 is not part of
 * the serialized string.
 */
static RD_INLINE RD_UNUSED
rd_kafkap_str_t *rd_kafkap_str_new (const char *str, int len) {
	rd_kafkap_str_t *kstr;
	int16_t klen;

	if (!str)
		len = RD_KAFKAP_STR_LEN_NULL;
	else if (len == -1)
		len = str ? (int)strlen(str) : RD_KAFKAP_STR_LEN_NULL;

	kstr = rd_malloc(sizeof(*kstr) + 2 +
			 (len == RD_KAFKAP_STR_LEN_NULL ? 0 : len + 1));
	kstr->len = len;

	/* Serialised format: 16-bit string length */
	klen = htobe16(len);
	memcpy(kstr+1, &klen, 2);

	/* Serialised format: non null-terminated string */
	if (len == RD_KAFKAP_STR_LEN_NULL)
		kstr->str = NULL;
	else {
		kstr->str = ((const char *)(kstr+1))+2;
		memcpy((void *)kstr->str, str, len);
		((char *)kstr->str)[len] = '\0';
	}

	return kstr;
}


/**
 * Makes a copy of `src`. The copy will be fully allocated and should
 * be freed with rd_kafka_pstr_destroy()
 */
static RD_INLINE RD_UNUSED
rd_kafkap_str_t *rd_kafkap_str_copy (const rd_kafkap_str_t *src) {
        return rd_kafkap_str_new(src->str, src->len);
}

static RD_INLINE RD_UNUSED int rd_kafkap_str_cmp (const rd_kafkap_str_t *a,
						 const rd_kafkap_str_t *b) {
	int minlen = RD_MIN(a->len, b->len);
	int r = memcmp(a->str, b->str, minlen);
	if (r)
		return r;
	else
		return a->len - b->len;
}

static RD_INLINE RD_UNUSED int rd_kafkap_str_cmp_str (const rd_kafkap_str_t *a,
						     const char *str) {
	int len = (int)strlen(str);
	int minlen = RD_MIN(a->len, len);
	int r = memcmp(a->str, str, minlen);
	if (r)
		return r;
	else
		return a->len - len;
}

static RD_INLINE RD_UNUSED int rd_kafkap_str_cmp_str2 (const char *str,
						      const rd_kafkap_str_t *b){
	int len = (int)strlen(str);
	int minlen = RD_MIN(b->len, len);
	int r = memcmp(str, b->str, minlen);
	if (r)
		return r;
	else
		return len - b->len;
}



/**
 *
 * Kafka protocol bytes array representation prefixed with a convenience header
 *
 * Serialized format:
 *  { uint32, data.. }
 *
 */
typedef struct rd_kafkap_bytes_s {
	/* convenience header (aligned access, host endian) */
	int32_t     len;   /* Kafka bytes length (-1=NULL, 0=empty, >0=data) */
	const void *data;  /* points just past the struct, or other memory,
			    * not NULL-terminated */
	const char _data[]; /* Bytes following struct when new()ed */
} rd_kafkap_bytes_t;


#define RD_KAFKAP_BYTES_LEN_NULL -1
#define RD_KAFKAP_BYTES_IS_NULL(kbytes) \
	((kbytes)->len == RD_KAFKAP_BYTES_LEN_NULL)

/* Returns the length of the bytes of a kafka protocol bytes representation */
#define RD_KAFKAP_BYTES_LEN0(len) ((len) == RD_KAFKAP_BYTES_LEN_NULL ? 0:(len))
#define RD_KAFKAP_BYTES_LEN(kbytes) RD_KAFKAP_BYTES_LEN0((kbytes)->len)

/* Returns the actual size of a kafka protocol bytes representation. */
#define RD_KAFKAP_BYTES_SIZE0(len) (4 + RD_KAFKAP_BYTES_LEN0(len))
#define RD_KAFKAP_BYTES_SIZE(kbytes) RD_KAFKAP_BYTES_SIZE0((kbytes)->len)


/* Serialized Kafka bytes: only works for _new() kbytes */
#define RD_KAFKAP_BYTES_SER(kbytes)  ((kbytes)+1)


/**
 * Frees a Kafka bytes previously allocated with `rd_kafkap_bytes_new()`
 */
static RD_UNUSED void rd_kafkap_bytes_destroy (rd_kafkap_bytes_t *kbytes) {
	rd_free(kbytes);
}


/**
 * Allocate a new Kafka bytes and make a copy of 'bytes'.
 * Supports Kafka NULL bytes.
 */
static RD_INLINE RD_UNUSED
rd_kafkap_bytes_t *rd_kafkap_bytes_new (const char *bytes, int32_t len) {
	rd_kafkap_bytes_t *kbytes;
	int32_t klen;

	if (!bytes)
		len = RD_KAFKAP_BYTES_LEN_NULL;

	kbytes = rd_malloc(sizeof(*kbytes) + 4 +
			 (len == RD_KAFKAP_BYTES_LEN_NULL ? 0 : len));
	kbytes->len = len;

	klen = htobe32(len);
	memcpy(kbytes+1, &klen, 4);

	if (len == RD_KAFKAP_BYTES_LEN_NULL)
		kbytes->data = NULL;
	else {
		kbytes->data = ((const char *)(kbytes+1))+4;
		memcpy((void *)kbytes->data, bytes, len);
	}

	return kbytes;
}


/**
 * Makes a copy of `src`. The copy will be fully allocated and should
 * be freed with rd_kafkap_bytes_destroy()
 */
static RD_INLINE RD_UNUSED
rd_kafkap_bytes_t *rd_kafkap_bytes_copy (const rd_kafkap_bytes_t *src) {
        return rd_kafkap_bytes_new(src->data, src->len);
}


static RD_INLINE RD_UNUSED int rd_kafkap_bytes_cmp (const rd_kafkap_bytes_t *a,
						   const rd_kafkap_bytes_t *b) {
	int minlen = RD_MIN(a->len, b->len);
	int r = memcmp(a->data, b->data, minlen);
	if (r)
		return r;
	else
		return a->len - b->len;
}

static RD_INLINE RD_UNUSED
int rd_kafkap_bytes_cmp_data (const rd_kafkap_bytes_t *a,
			      const char *data, int len) {
	int minlen = RD_MIN(a->len, len);
	int r = memcmp(a->data, data, minlen);
	if (r)
		return r;
	else
		return a->len - len;
}




typedef struct rd_kafka_buf_s rd_kafka_buf_t;


#define RD_KAFKA_NODENAME_SIZE  128

