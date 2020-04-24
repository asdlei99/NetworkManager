// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2018 Red Hat, Inc.
 */

#include "nm-default.h"

#include "nm-sd-utils-shared.h"

#include "nm-sd-adapt-shared.h"

#include "dns-domain.h"
#include "hexdecoct.h"
#include "hostname-util.h"
#include "path-util.h"
#include "web-util.h"

/*****************************************************************************/

const bool mempool_use_allowed = true;

/*****************************************************************************/

gboolean
nm_sd_utils_path_equal (const char *a, const char *b)
{
	return path_equal (a, b);
}

char *
nm_sd_utils_path_simplify (char *path, gboolean kill_dots)
{
	return path_simplify (path, kill_dots);
}

const char *
nm_sd_utils_path_startswith (const char *path, const char *prefix)
{
	return path_startswith (path, prefix);
}

/*****************************************************************************/

int
nm_sd_utils_unbase64char (char ch, gboolean accept_padding_equal)
{
	if (   ch == '='
	    && accept_padding_equal)
		return G_MAXINT;
	return unbase64char (ch);
}

/**
 * nm_sd_utils_unbase64mem:
 * @p: a valid base64 string. Whitespace is ignored, but invalid encodings
 *   will cause the function to fail.
 * @l: the length of @p. @p is not treated as NUL terminated string but
 *   merely as a buffer of ascii characters.
 * @secure: whether the temporary memory will be cleared to avoid leaving
 *   secrets in memory (see also nm_explict_bzero()).
 * @mem: (transfer full): the decoded buffer on success.
 * @len: the length of @mem on success.
 *
 * glib provides g_base64_decode(), but that does not report any errors
 * from invalid encodings. Expose systemd's implementation which does
 * reject invalid inputs.
 *
 * Returns: a non-negative code on success. Invalid encoding let the
 *   function fail.
 */
int
nm_sd_utils_unbase64mem (const char *p,
                         size_t l,
                         gboolean secure,
                         guint8 **mem,
                         size_t *len)
{
	return unbase64mem_full (p, l, secure, (void **) mem, len);
}

int nm_sd_dns_name_to_wire_format (const char *domain,
                                   guint8 *buffer,
                                   size_t len,
                                   gboolean canonical)
{
	return dns_name_to_wire_format (domain, buffer, len, canonical);
}

int nm_sd_dns_name_is_valid (const char *s)
{
	return dns_name_is_valid (s);
}

gboolean nm_sd_hostname_is_valid (const char *s, bool allow_trailing_dot)
{
	return hostname_is_valid (s, allow_trailing_dot);
}

/*****************************************************************************/

static gboolean
_http_url_is_valid (const char *url)
{
	if (   !url
	    || !url[0])
		return FALSE;

	if (NM_STR_HAS_PREFIX (url, "http://"))
		url += NM_STRLEN ("http://");
	else if (NM_STR_HAS_PREFIX (url, "https://"))
		url += NM_STRLEN ("https://");
	else
		return FALSE;

	if (!url[0])
		return FALSE;

	return !NM_STRCHAR_ANY (url, ch, (guchar) ch >= 128u);
}

gboolean
nm_sd_http_url_is_valid (const char *url)
{
	gboolean v;

	/* http_url_is_valid() is part of our API, as we use it to validate connection
	 * properties. That means, it's behavior must remain stable (or only change
	 * with care).
	 *
	 * Thus, reimplement it, and make sure that our implementation agrees. */
	v = _http_url_is_valid (url);
	nm_assert (v == http_url_is_valid (url));
	return v;
}
