#pragma once

/*
 * Summit Tale server compatibility layer.
 *
 * Initializes the legacy LastCategory -> CODE table used by
 * DecodeItemInfo/OpenItem parsing.
 *
 * Kept separate from sinItem.h because the legacy client and server
 * sinItem.h files share the same _SINITEM_HEADER_ include guard.
 */
int InitializeServerItemCodeMap();
