//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_REQUESTS_IMPL_MIME_TYPES_IPP
#define BOOST_REQUESTS_IMPL_MIME_TYPES_IPP

#include <boost/requests/mime_types.hpp>

namespace boost {
namespace requests {


const mime_type_map & default_mime_type_map()
{
  // copied from https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
  const static mime_type_map mp
      {
        { ".3g2", /*3GPP2 audio/video container*/ "video/3gpp2"},
        { ".7z", /*7-zip archive*/	"application/x-7z-compressed"},
        { ".aac", /*AAC audio*/ "audio/aac"},
        { ".abw", /*AbiWord document*/ "application-x-abiword"},
        { ".arc", /*Archive document (multiple files embedded)*/	"application-x-freearc"},
        { ".avif", /*AVIF image*/ "image/avif"},
        { ".avi", /*AVI: Audio Video Interleave	*/ "videox-msvideo"},
        { ".azw", /*Amazon Kindle eBook format */	"application/vnd.amazon.ebook"},
        { ".bin", /*Any kind of binary data	*/ "application/octet-stream"},
        { ".bmp", /*Windows OS/2 Bitmap Graphics*/ "image/bmp"},
        { ".bz", /*BZip archive	*/ "application/x-bzip"},
        { ".bz2", /*BZip2 archive	*/ "application/x-bzip2"},
        { ".cda", /*CD audio	*/ "application/x-cdf"},
        { ".csh", /*C-Shell script	*/ "application/x-csh"},
        { ".css", /*Cascading Style Sheets (CSS)*/"text/css"},
        { ".csv", /*Comma-separated values (CSV)*/"text/csv"},
        { ".doc", /*Microsoft Word*/ "application/msword"},
        { ".docx", /*Microsoft Word (OpenXML)	*/ "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        { ".eot", /*MS Embedded OpenType fonts	*/ "application/vnd.ms-fontobject"},
        { ".epub", /*Electronic publication (EPUB)	*/ "application/epub+zip"},
        { ".gz", /*GZip Compressed Archive*/ "application/gzip"},
        { ".gif", /*Graphics Interchange Format (GIF)*/"image/gif"},
        { ".htm", /*.html	HyperText Markup Language (HTML)*/"text/html"},
        { ".html", /*.html	HyperText Markup Language (HTML)*/"text/html"},
        { ".ico", /*Icon format*/ "image/vnd"},
        { ".ics", /*iCalendar format*/ "text/calendar"},
        { ".jar", /*Java Archive (JAR)	*/ "application/java-archive"},
        { ".jpeg", /*.jpg	JPEG images*/ "image/jpeg"},
        { ".js", /*JavaScript	*/"text/javascript"/* (Specifications: HTML and "RFC 9239)"*/},
        { ".json", /*JSON format*/ "application/json"},
        { ".jsonld", /*JSON-LD format	*/ "application/ld+json"},
        { ".mid", /*.midi	Musical Instrument Digital Interface (MIDI)	audio/midi */ "audio/x-midi"},
        { ".mjs", /*JavaScript module*/ "text/javascript"},
        { ".mp3", /*MP3 audio*/ "audio/mpeg"},
        { ".mp4", /*MP4 video*/ "video/mp4"},
        { ".mpeg", /*MPEG Video*/ "video/mpeg"},
        { ".mpkg", /*Apple Installer Package */	"application/vnd.apple.installer+xml"},
        { ".odp", /*OpenDocument presentation document */ "application/vnd.oasis.opendocument.presentation"},
        { ".ods", /*OpenDocument spreadsheet document  */ "application/vnd.oasis.opendocument.spreadsheet"},
        { ".odt", /*OpenDocument text document	       */ "application/vnd.oasis.opendocument.text"},
        { ".oga", /*OGG audio*/ "audio/ogg"},
        { ".ogv", /*OGG video*/ "video/ogg"},
        { ".ogx", /*OGG*/ "application/ogg"},
        { ".opus", /*Opus audio*/ "audio/opus"},
        { ".otf", /*OpenType font*/ "font/otf"},
        { ".png", /*Portable Network Graphics*/ "image/png"},
        { ".pdf", /*Adobe Portable Document Format (PDF)*/"application/pdf"},
        { ".php", /*Hypertext Preprocessor (Personal Home Page)	application/x*/ "httpd-php"},
        { ".ppt", /*Microsoft PowerPoint	application/vnd*/ "ms-powerpoint"},
        { ".pptx", /*Microsoft PowerPoint (OpenXML)	*/ "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        { ".rar", /*RAR archive	*/ "application/vnd.rar"},
        { ".rtf", /*Rich Text Format (RTF)*/"application/rtf"},
        { ".sh", /*Bourne shell script	*/ "application/x-sh"},
        { ".svg", /*Scalable Vector Graphics (SVG)	*/ "image/svg+xml"},
        { ".tar", /*Tape Archive (TAR)	*/ "application/x-tar"},
        { ".tif", /*.tiff	Tagged Image File Format (TIFF)*/"image/tiff"},
        { ".tiff", /*.tiff	Tagged Image File Format (TIFF)*/"image/tiff"},
        { ".ts", /*MPEG transport stream*/ "video/mp2t"},
        { ".ttf", /*TrueType Font*/ "font/ttf"},
        { ".txt", /*Text, (generally ASCII or ISO 8859-n)*/"text/plain"},
        { ".vsd", /*	Microsoft Visio	*/ "application/vnd.visio"},
        { ".wav", /*Waveform Audio Format*/ "audio/wav"},
        { ".weba", /*WEBM audio*/ "audio/webm"},
        { ".webm", /*WEBM video*/ "video/webm"},
        { ".webp", /*WEBP image*/ "image/webp"},
        { ".woff", /*Web Open Font Format (WOFF)*/"font/woff"},
        { ".woff2", /*Web Open Font Format (WOFF)*/"font/woff2"},
        { ".xhtml", /*XHTML	*/ "application/xhtml+xml"},
        { ".xls", /*Microsoft Excel	application/vnd*/ "ms-excel"},
        { ".xlsx", /*Microsoft Excel (OpenXML) */	"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        { ".xml", /*  */ "application/xml"/* serves as a valid default.*/ }
      };
  return mp;
}


}
}

#endif // BOOST_REQUESTS_IMPL_MIME_TYPES_IPP
