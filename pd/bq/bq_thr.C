// This file is part of the pd::bq library.
// Copyright (C) 2006-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2006-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "bq_thr_impl.I"

#include <pd/base/exception.H>

namespace pd {

bool bq_success(bq_err_t err) throw() {
	switch(err) {
		case bq_ok: return true;
		case bq_timeout: errno = ETIMEDOUT; break;
		case bq_not_available: errno = ECANCELED; break;
		case bq_overload: errno = EBUSY; break;
		case bq_illegal_call: errno = EBADSLT; break;
	}
	return false;
}

void bq_thr_t::init(
	size_t _maxevs, interval_t _timeout, bq_cont_count_t &cont_count,
	string_t const &tname, bq_post_activate_t *post_activate
) {
	assert(!impl);

	impl = new impl_t(_maxevs, _timeout, cont_count, post_activate);

	impl->init(tname);
}

void bq_thr_t::stop() { impl_t::work = false; }

void bq_thr_t::fini() {
	assert(impl);

	impl->fini();

	delete impl;
	impl = NULL;
}

bq_cont_count_t &bq_thr_t::cont_count() throw() {
	assert(impl);

	return impl->cont_count;
}

stat::mmcount_t &bq_thr_t::stat_conts() throw() {
	assert(impl);

	return impl->stat.conts();
}

pid_t bq_thr_t::get_tid() const throw() {
	assert(impl);

	return impl->tid;
}

void bq_thr_t::stat_print() {
	assert(impl);

	impl->stat_print();
}

class switch_item_t : public bq_thr_t::impl_t::item_t {
	virtual void attach() throw();
	virtual void detach() throw();

public:
	inline switch_item_t(interval_t *timeout) throw() : item_t(timeout, true) { }

	inline ~switch_item_t() throw() { }
};

void switch_item_t::attach() throw() { impl->poke(); }

void switch_item_t::detach() throw() { }

bq_err_t bq_thr_t::switch_to(interval_t const &prio, bool force) {
	assert(impl);

	if(this != bq_thr_get() && !bq_thr_set(this))
		return bq_overload;

	if(!force && impl == impl_t::current)
		return bq_ok;

	interval_t timeout = prio;
	switch_item_t item(&timeout);
	return item.suspend("switching");
}

} // namespace pd
