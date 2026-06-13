#ifndef __simpl_http_h__
#define __simpl_http_h__

#include <simpl/library.h>
#include <simpl/value.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace simpl
{
	namespace detail
	{
		inline std::string default_content_type(const std::string& method, const std::string& body)
		{
			if (!body.empty() && method == "POST")
			{
				return "application/json";
			}

			return {};
		}

		inline std::string get_http_string(const blob_t& request, const std::string& key)
		{
			const auto found = request.values.find(key);
			if (found == request.values.end())
			{
				return {};
			}

			return cast<std::string>(found->second);
		}

		inline std::string require_http_string(const blob_t& request, const std::string& key)
		{
			const auto found = request.values.find(key);
			if (found == request.values.end())
			{
				throw std::runtime_error("http request missing '" + key + "'");
			}

			return cast<std::string>(found->second);
		}

#ifdef _WIN32
		class winhttp_handle final
		{
		public:
			winhttp_handle(HINTERNET handle = nullptr)
				: handle_(handle)
			{
			}

			~winhttp_handle()
			{
				if (handle_ != nullptr)
				{
					::WinHttpCloseHandle(handle_);
				}
			}

			winhttp_handle(const winhttp_handle&) = delete;
			winhttp_handle& operator=(const winhttp_handle&) = delete;

			winhttp_handle(winhttp_handle&& rhs) noexcept
				: handle_(rhs.handle_)
			{
				rhs.handle_ = nullptr;
			}

			winhttp_handle& operator=(winhttp_handle&& rhs) noexcept
			{
				if (this == &rhs)
				{
					return *this;
				}

				if (handle_ != nullptr)
				{
					::WinHttpCloseHandle(handle_);
				}

				handle_ = rhs.handle_;
				rhs.handle_ = nullptr;
				return *this;
			}

			HINTERNET get() const
			{
				return handle_;
			}

			explicit operator bool() const
			{
				return handle_ != nullptr;
			}

		private:
			HINTERNET handle_;
		};

		struct parsed_http_url
		{
			std::wstring host;
			std::wstring path;
			INTERNET_PORT port = 0;
			bool secure = false;
		};

		inline std::runtime_error make_http_error(const char* action)
		{
			std::ostringstream ss;
			ss << action << " failed (" << ::GetLastError() << ")";
			return std::runtime_error(ss.str());
		}

		inline std::wstring to_wide(const std::string& input)
		{
			if (input.empty())
			{
				return std::wstring{};
			}

			const int chars = ::MultiByteToWideChar(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);
			if (chars <= 0)
			{
				throw make_http_error("MultiByteToWideChar");
			}

			std::wstring output(static_cast<size_t>(chars), L'\0');
			const int converted = ::MultiByteToWideChar(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), output.data(), chars);
			if (converted != chars)
			{
				throw make_http_error("MultiByteToWideChar");
			}

			return output;
		}

		inline parsed_http_url parse_http_url(const std::string& url)
		{
			const auto wurl = to_wide(url);

			URL_COMPONENTS components{};
			components.dwStructSize = sizeof(components);
			components.dwSchemeLength = static_cast<DWORD>(-1);
			components.dwHostNameLength = static_cast<DWORD>(-1);
			components.dwUrlPathLength = static_cast<DWORD>(-1);
			components.dwExtraInfoLength = static_cast<DWORD>(-1);

			if (!::WinHttpCrackUrl(wurl.c_str(), static_cast<DWORD>(wurl.size()), 0, &components))
			{
				throw std::runtime_error("invalid URL");
			}

			if (components.nScheme != INTERNET_SCHEME_HTTP && components.nScheme != INTERNET_SCHEME_HTTPS)
			{
				throw std::runtime_error("URL scheme must be http or https");
			}

			parsed_http_url result;
			result.host.assign(components.lpszHostName, components.dwHostNameLength);
			result.path.assign(components.lpszUrlPath, components.dwUrlPathLength);
			if (components.dwExtraInfoLength > 0)
			{
				result.path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
			}
			if (result.path.empty())
			{
				result.path = L"/";
			}
			result.port = components.nPort;
			result.secure = components.nScheme == INTERNET_SCHEME_HTTPS;
			return result;
		}

		inline blobref_t send_http_request(const std::string& method, const std::string& url, const std::string& body, const std::string& content_type)
		{
			const auto parsed = parse_http_url(url);
			const auto wmethod = to_wide(method);

			winhttp_handle session(::WinHttpOpen(
				L"simpl-http/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0));
			if (!session)
			{
				throw make_http_error("WinHttpOpen");
			}

			winhttp_handle connect(::WinHttpConnect(session.get(), parsed.host.c_str(), parsed.port, 0));
			if (!connect)
			{
				throw make_http_error("WinHttpConnect");
			}

			const DWORD flags = parsed.secure ? WINHTTP_FLAG_SECURE : 0;
			winhttp_handle request(::WinHttpOpenRequest(
				connect.get(),
				wmethod.c_str(),
				parsed.path.c_str(),
				nullptr,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				flags));
			if (!request)
			{
				throw make_http_error("WinHttpOpenRequest");
			}

			std::wstring headers;
			if (!content_type.empty())
			{
				headers = L"Content-Type: " + to_wide(content_type) + L"\r\n";
			}

			LPVOID body_data = body.empty() ? WINHTTP_NO_REQUEST_DATA : static_cast<LPVOID>(const_cast<char*>(body.data()));
			DWORD body_length = static_cast<DWORD>(body.size());

			if (!::WinHttpSendRequest(
				request.get(),
				headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
				headers.empty() ? 0 : static_cast<DWORD>(-1),
				body_data,
				body_length,
				body_length,
				0))
			{
				throw make_http_error("WinHttpSendRequest");
			}

			if (!::WinHttpReceiveResponse(request.get(), nullptr))
			{
				throw make_http_error("WinHttpReceiveResponse");
			}

			DWORD status = 0;
			DWORD status_size = sizeof(status);
			if (!::WinHttpQueryHeaders(
				request.get(),
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&status,
				&status_size,
				WINHTTP_NO_HEADER_INDEX))
			{
				throw make_http_error("WinHttpQueryHeaders");
			}

			std::string response_body;
			for (;;)
			{
				DWORD available = 0;
				if (!::WinHttpQueryDataAvailable(request.get(), &available))
				{
					throw make_http_error("WinHttpQueryDataAvailable");
				}

				if (available == 0)
				{
					break;
				}

				std::vector<char> chunk(static_cast<size_t>(available));
				DWORD bytes_read = 0;
				if (!::WinHttpReadData(request.get(), chunk.data(), available, &bytes_read))
				{
					throw make_http_error("WinHttpReadData");
				}

				response_body.append(chunk.data(), chunk.data() + bytes_read);
			}

			auto result = new_blob();
			result->values["status"] = number{ static_cast<double>(status) };
			result->values["body"] = response_body;
			return result;
		}
#endif

		inline blobref_t request_http(const std::string& method, const std::string& url, const std::string& body, const std::string& content_type)
		{
#ifdef _WIN32
			return send_http_request(method, url, body, content_type);
#else
			(void)method;
			(void)url;
			(void)body;
			(void)content_type;
			throw std::runtime_error("http library is only supported on Windows");
#endif
		}

		inline blobref_t request_http(const std::string& url)
		{
			return request_http("GET", url, std::string{}, std::string{});
		}

		inline blobref_t request_http(const std::string& method, const std::string& url)
		{
			return request_http(method, url, std::string{}, std::string{});
		}

		inline blobref_t request_http(const std::string& method, const std::string& url, const std::string& body)
		{
			return request_http(method, url, body, default_content_type(method, body));
		}

		inline blobref_t request_http(const blob_t& request)
		{
			const auto body = get_http_string(request, "body");
			auto method = get_http_string(request, "method");
			if (method.empty())
			{
				method = body.empty() ? "GET" : "POST";
			}

			auto content_type = get_http_string(request, "content_type");
			if (content_type.empty())
			{
				content_type = default_content_type(method, body);
			}

			return request_http(method, require_http_string(request, "url"), body, content_type);
		}
	}

	class http_lib final : public library
	{
	public:
		const char* name() const override
		{
			return "http";
		}

		void load(vm& vm) override
		{
			vm.reg_fn("request", [](const std::string& url)
			{
				return detail::request_http(url);
			});

			vm.reg_fn("request", [](const std::string& method, const std::string& url)
			{
				return detail::request_http(method, url);
			});

			vm.reg_fn("request", [](const std::string& method, const std::string& url, const std::string& body)
			{
				return detail::request_http(method, url, body);
			});

			vm.reg_fn("request", [](const std::string& method, const std::string& url, const std::string& body, const std::string& content_type)
			{
				return detail::request_http(method, url, body, content_type);
			});

			vm.reg_fn("request", [](const blob_t& request)
			{
				return detail::request_http(request);
			});

			vm.reg_fn("get", [](const std::string& url)
			{
				return detail::request_http(url);
			});

			vm.reg_fn("post", [](const std::string& url, const std::string& body)
			{
				return detail::request_http("POST", url, body);
			});

			vm.reg_fn("post", [](const std::string& url, const std::string& body, const std::string& content_type)
			{
				return detail::request_http("POST", url, body, content_type);
			});
		}
	};
}

#endif //__simpl_http_h__
