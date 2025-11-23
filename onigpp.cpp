// onigpp.cpp --- Oniguruma++ (onigpp) implementation
// Author: katahiromz
// License: MIT

#include "onigpp.h"

namespace onigpp {

////////////////////////////////////////////
// Implementation of regex_error

regex_error::regex_error(regex_constants::error_type ecode, const OnigErrorInfo& err_info)
	: m_err_code(ecode), m_err_info(err_info)
{
}

regex_error::~regex_error() { }

regex_constants::error_type regex_error::code() const {
	return m_err_code;
}

const char* regex_error::what() const noexcept {
	static thread_local char err_buf[ONIG_MAX_ERROR_MESSAGE_LEN];
	if (onig_is_error_code_needs_param(m_err_code)) {
		onig_error_code_to_str((OnigUChar*)err_buf, m_err_code, &m_err_info);
	} else {
		onig_error_code_to_str((OnigUChar*)err_buf, m_err_code);
	}
	return err_buf;
}

////////////////////////////////////////////
// onigpp::init

int init(const OnigEncoding *encodings, size_t encodings_count) {
	static OnigEncoding use_encodings[] = {
#define SUPPORTED_ENCODING(enc) enc,
#include "encodings.h"
#undef SUPPORTED_ENCODING
	};
	if (!encodings) {
		encodings = use_encodings;
		encodings_count = sizeof(use_encodings) / sizeof(use_encodings[0]);
	}
	int err = onig_initialize((OnigEncoding *)encodings, (int)encodings_count);
	if (err != ONIG_NORMAL) {
		throw std::runtime_error("onig_initialize failed");
	}
	return err;
}

////////////////////////////////////////////
// onigpp::uninit

void uninit() { onig_end(); }

////////////////////////////////////////////
// onigpp::version

const char* version() { return onig_version(); }

} // namespace onigpp
