/* *
 *	from https://github.com/nathanwiegand/urlparser
 * */
#ifndef __URL_PARSER__
#define __URL_PARSER__

/**
 * This library implements the following regular expression from    
 * http://tools.ietf.org/html/rfc3986
 *
 * /^(([^:\/?#]+):)?(\/\/([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?/
 *   12              3   4           5       6  7        8 9
 *
*/


/* TODO(nathan): Should probably make this opaque. Also, should this be renamed?
 */

/**
 *URLGroup defines the start and end points of a string within the larger URL.
 *Analagous to the endpoints of a grouping in a Perl regex. 
 */
typedef struct {
  int start;
  int end;
} URLGroup;


/**
 * URL is a struct which holds the end points of all of the matchings.  These
 * are only end-points not strings so that we can lazily convert them to strings
 * to save time and memory.  NOTE: When used in the context of 'parseURLField'
 * any field *after* the field you desired is guaranteed to have endpoints 0,0.
 */
typedef struct {
  /**
   * e.g. 'http' in 'http://www.google.com/some/path?query=foo#place-in-page'
   */
  URLGroup scheme;     
  /**
   * e.g. 'www.google.com' in 
   *    'http://www.google.com/some/path?query=foo#place-in-page'
   */
  URLGroup authority;
  /**
   * e.g. '/some/path' in 
   *    'http://www.google.com/some/path?query=foo#place-in-page'
   */
  URLGroup path;
  /**
   * e.g. 'query=foo' in 
   *    'http://www.google.com/some/path?query=foo#place-in-page'
   */
  URLGroup query;
  /**
   * e.g. 'place-in-page' in 
   *     'http://www.google.com/some/path?query=foo#place-in-page'
   */
  URLGroup fragment;
} URL;

/**
 * URLField is an enum which corresponds to the group numbers in the regular
 * expression from above.  Used in specifying which field to stop at in
 * 'parseURLField'
 */
typedef enum {
  URL_SCHEME    = 2,
  URL_AUTHORITY = 4,
  URL_PATH      = 5,
  URL_QUERY     = 7,
  URL_FRAGMENT  = 9
} URLField;

/**
 * This function takes a string representing a url and a URL struct.  It then
 * parses the entire url by calling 'parseURLField' with third parameter 0, 
 * The result is that the end-points of each substring are inserted into the
 * struct.  Note that the struct contains only the end-points, not strings.
 * This is done to save space and time.  Also, note, IF NO SCHEME WAS SPECIFIED
 * THEN 'www.google.com' WILL BE STORED IN 'URL_PATH' *NOT* 'URL_AUTHORITY'.  It
 * is *your* responsibiltiy to have a default scheme and respond to no scheme
 * being presented.
 * @param url A const pointer to the string representing the url
 * @param storage The location of the URL struct for the values to be stored in.
 * @return Returns 0 on success, non-zero on failuer.
 */
int parseURL(const char *url, URL *storage);

/**
 * This function takes a string representing a url and a URL struct.  It then
 * parses the entire url up to the field specified by 'field' in the last
 * parameter.  The result is that the end-points of each substring are inserted 
 * into the  struct.  Note that the struct contains only the end-points, not 
 * strings. This is done to save space and time.  Also, note, IF NO SCHEME WAS 
 * SPECIFIED THEN 'www.google.com' WILL BE STORED IN 'URL_PATH' *NOT* 
 * 'URL_AUTHORITY'.  It is *your* responsibiltiy to have a default scheme and 
 * respond to no scheme being presented.  All fields after the field you
 * specified are guaranteed to have endponts 0,0.
 * @param url A const pointer to the string representing the url
 * @param storage The location of the URL struct for the values to be stored in.
 * @param field The field to stop parsing at.  For example, if you only want the
 * scheme then provlde URL_SCHEME.
 * @return Returns 0 on success, non-zero on failuer.
 */
int parseURLField(const char *url, URL *storage, URLField field);

/**
 * Given a string representing a url and the endpoints of the substring, convert
 * them into their own string.  This, in effect, evaluates the lazy substring
 * generated by the 'parseURLField' call. 
 * @param url The const pointer to the string representing the url
 * @param field The 'URLGroup' holding the endpoints of the desired substring.
 * @return Returns a pointer to the substring
 */
char *readURLField(const char *url, URLGroup field);

/**
 * Compare the value of one of the fields in the string to some input.  Works
 * like 'strcmp' in that it returns 0 on success, non-zero otherwise.  This is
 * needed when computing the lazy functions since there are no mem-copies.
 * @param url A const pointer to the string containing your url
 * @param field the 'URLGroup' holding the endpoints of your substring
 * @param to the string to compare to
 * @return 0 if the strings are equal, non-zero otherwise.
 */
int cmpURLField(const char *url, URLGroup field, const char *to);
#endif