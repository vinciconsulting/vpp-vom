/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vom/l3_binding.hpp"
#include "vom/l3_binding_cmds.hpp"
#include "vom/singular_db_funcs.hpp"

namespace VOM {
singular_db<l3_binding::key_t, l3_binding> l3_binding::m_db;

l3_binding::event_handler l3_binding::m_evh;

/**
 * Construct a new object matching the desried state
 */
l3_binding::l3_binding(const interface& itf, const route::prefix_t& pfx)
  : m_itf(itf.singular())
  , m_pfx(pfx)
  , m_binding(true, rc_t::NOOP)
{
}

l3_binding::l3_binding(const l3_binding& o)
  : m_itf(o.m_itf)
  , m_pfx(o.m_pfx)
  , m_binding(o.m_binding)
{
}

l3_binding::~l3_binding()
{
  sweep();

  // not in the DB anymore.
  m_db.release(key(), this);
}

bool
l3_binding::operator==(const l3_binding& l) const
{
  return ((m_pfx == l.m_pfx) && (*m_itf == *l.m_itf));
}

const l3_binding::key_t
l3_binding::key() const
{
  return (make_pair(m_itf->key(), m_pfx));
}

void
l3_binding::sweep()
{
  if (m_binding) {
    HW::enqueue(
      new l3_binding_cmds::unbind_cmd(m_binding, m_itf->handle(), m_pfx));
  }
  HW::write();
}

void
l3_binding::replay()
{
  if (m_binding) {
    HW::enqueue(
      new l3_binding_cmds::bind_cmd(m_binding, m_itf->handle(), m_pfx));
  }
}

const route::prefix_t&
l3_binding::prefix() const
{
  return (m_pfx);
}

const interface&
l3_binding::itf() const
{
  return (*m_itf);
}

l3_binding::const_iterator_t
l3_binding::cbegin()
{
  return m_db.begin();
}

l3_binding::const_iterator_t
l3_binding::cend()
{
  return m_db.end();
}

std::string
l3_binding::to_string() const
{
  std::ostringstream s;
  s << "L3-binding:[" << m_itf->to_string() << " prefix:" << m_pfx.to_string()
    << " " << m_binding.to_string() << "]";

  return (s.str());
}

void
l3_binding::update(const l3_binding& desired)
{
  /*
   * no updates for the binding. chaning the interface or the prefix is a change
   * to the
   * key, hence a new object
   */
  if (!m_binding) {
    HW::enqueue(
      new l3_binding_cmds::bind_cmd(m_binding, m_itf->handle(), m_pfx));
  }
}

std::shared_ptr<l3_binding>
l3_binding::find_or_add(const l3_binding& temp)
{
  return (m_db.find_or_add(temp.key(), temp));
}

std::shared_ptr<l3_binding>
l3_binding::find(const key_t& k)
{
  return (m_db.find(k));
}

std::shared_ptr<l3_binding>
l3_binding::singular() const
{
  return find_or_add(*this);
}

void
l3_binding::dump(std::ostream& os)
{
  db_dump(m_db, os);
}

std::ostream&
operator<<(std::ostream& os, const l3_binding::key_t& key)
{
  os << "[" << key.first << ", " << key.second << "]";

  return (os);
}

l3_binding::event_handler::event_handler()
{
  OM::register_listener(this);
  inspect::register_handler({ "l3" }, "L3 bindings", this);
}

void
l3_binding::event_handler::handle_replay()
{
  m_db.replay();
}

void
l3_binding::event_handler::handle_populate(const client_db::key_t& key)
{
  /**
   * This is done while populating the interfaces
   */
}

dependency_t
l3_binding::event_handler::order() const
{
  return (dependency_t::BINDING);
}

void
l3_binding::event_handler::show(std::ostream& os)
{
  db_dump(m_db, os);
}
}

/*
 * fd.io coding-style-patch-verification: OFF
 *
 * Local Variables:
 * eval: (c-set-style "mozilla")
 * End:
 */
