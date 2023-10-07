#include "Config/ErrorPage.hpp"

ErrorPage::ErrorPage()
	: path(), media_type("application/octet-stream")
{
}

ErrorPage::ErrorPage(const std::string &path)
	: path(path), media_type("application/octet-stream")
{
}

ErrorPage::ErrorPage(const std::string &path, const std::string &media_type)
	: path(path), media_type(media_type)
{
}
