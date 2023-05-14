#include <iostream>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "exception.hpp"

using namespace std;
using ptree = boost::property_tree::ptree;

#define TOKEN_DTA_HREF "<DT><A HREF="
#define TOKEN_A_CLOSER "</A>"
#define TAGS_DELIMETER " "

map<string, string> UrlsMap;
const ptree EmptyTree;
string TagPrefix = TAGS_DELIMETER;

/// @brief prefix Tags
/// @param tags: string with comma delimeter
/// @return string: prefixed tags delimited by TAGS_DELIMETER
string prefixTags(const string& tags) {
  string tagsPrefixed;
  if (!tags.empty()) {
    size_t commaPos=0, nextCommaPos;
    do {
      nextCommaPos = tags.find(',', commaPos);
      tagsPrefixed += TagPrefix + tags.substr(commaPos, nextCommaPos-commaPos);
      commaPos=nextCommaPos+1;
    } while (nextCommaPos != string::npos);
  }
  return tagsPrefixed;
}

/// @brief recursive fill UrlsMap with urls and its prefixed tags
/// @param jtree 
void mapUrlsFromJson(const ptree& jtree) {
    const auto& uri = jtree.get<string>("uri", "");
    if (!uri.empty()) {
      const auto& tags = jtree.get<string>("tags", "");
      auto result = UrlsMap.insert(make_pair(uri, prefixTags(tags)));
      if (!result.second) { cout << "found doubled url: " << uri << endl; }
    }
    else {
      const ptree& childs = jtree.get_child("children", EmptyTree);
      for (const auto& child: childs) {
        mapUrlsFromJson(child.second);
      }
    }
}

/// @brief reTag Bookmarks Html File
/// @param string srcFName
/// @return int: count of urls processed
int reTagBookmarksHtmlFile(const string& srcFName) {
    string taggedFName(srcFName.substr(0, srcFName.find(".html")).append("-tagged.html"));

    std::ifstream src(srcFName);
    if (!src.is_open()) return -1;
    std::ofstream dst(taggedFName);
    if (!dst.is_open()) {
      src.close();
      return -1;
    }

    int linecounter=0, urlcounter=0;
    std::string line;
    while (std::getline(src, line)) {
        linecounter++;
        auto hrefPos = line.find(TOKEN_DTA_HREF);
        if (hrefPos == string::npos) {
          dst << line << endl;
        }
        else {
          urlcounter++;
          size_t urlPos = hrefPos+sizeof(TOKEN_DTA_HREF);
          auto endUrlPos = line.find("\"", urlPos);
          if (endUrlPos == string::npos) { //if not found a close quote then write back original line...
            dst << line << endl;
            cout << "WARN: not found close quote at line:" << linecounter << " : "<< line << endl;
          }
          else {
            string url(line.substr(urlPos, endUrlPos-urlPos));
            auto found = UrlsMap.find(url);
            if (found != UrlsMap.end()) {
              auto firstTagPos = line.find(TagPrefix, endUrlPos);
              int saveLineSize = firstTagPos == string::npos ? line.length()-sizeof(TOKEN_A_CLOSER): firstTagPos;
              string lineBeforePrefix(line.substr(0, saveLineSize));
              dst << lineBeforePrefix << (*found).second << TOKEN_A_CLOSER << endl;
            }
            else { // if not found url then simple copy back the line...
              dst << line << endl;
            }
          }
        }
    }
    dst.close();
    src.close();
    return urlcounter;
}

int main(int argc, char **argv) {
    try {
      if (argc < 3) {
          printf("Usage: %s <FF_bookmarks.json> <Chrome_bookmarks.html> [tag_prefix='#_']\n", argv[0]);
          return EXIT_FAILURE;
      }
      string jsonFName, htmlFName;
      jsonFName.assign(argv[1]);
      htmlFName.assign(argv[2]);
      if (argc > 3) TagPrefix.append(argv[3]); else TagPrefix.append("#_");

      printf("%s starts with %d params:\n", argv[0], argc);
      printf("1: JSON: %s\n", jsonFName.c_str());
      printf("2: HTML: %s\n", htmlFName.c_str());
      printf("3: Tag usage: %s\n", TagPrefix.c_str());

      ptree jtree;
      read_json(jsonFName, jtree);

      mapUrlsFromJson(jtree);

      int urlCounter = reTagBookmarksHtmlFile(htmlFName);
      cout << endl << "Succefully processed urls:" << urlCounter << endl;
    }
    catch (const std::runtime_error &ex) {
      cout << ex.what() << std::endl;
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
