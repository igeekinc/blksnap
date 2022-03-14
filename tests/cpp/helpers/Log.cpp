/*
 * Copyright (C) 2022 Veeam Software Group GmbH <https://www.veeam.com/contacts.html>
 *
 * This file is part of blksnap-tests
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Log.h"

#include <atomic>
#include <ctime>
#include <iomanip>
#include <thread>

static void buf2stream(const void* buf, const size_t size, std::stringstream& ss)
{
    int inx = 0;

    ss << std::hex << std::setfill('0') ;
    do {
        unsigned int value = static_cast<const unsigned char*>(buf)[inx];

        ss << std::setw(2) << value << " ";

        inx++;

        if (!(inx % 16))
            ss << std::endl;
        else if (!(inx % 8))
            ss << " ";
    } while (inx < size);
};

void CLog::Open(const std::string& filename)
{
    std::lock_guard<std::mutex> guard(m_lock);

    m_out.open(filename, std::ios::trunc);
    m_isOpen = true;
};

void CLog::Info(const char* message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cout << message << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << message << std::endl;
};

void CLog::Info(const std::string& message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cout << message << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << message << std::endl;
};

void CLog::Info(const std::stringstream& ss)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cout << ss.str() << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << ss.str() << std::endl;
};

void CLog::Info(const void* buf, const size_t size)
{
    std::stringstream ss;

    buf2stream(buf, size, ss);
    Info(ss);
}

void CLog::Err(const char* message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cerr << message << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " ERR " << message << std::endl;
};

void CLog::Err(const std::string& message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cerr << message << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " ERR " << message << std::endl;
};

void CLog::Err(const std::stringstream& ss)
{
    std::lock_guard<std::mutex> guard(m_lock);

    std::cerr << ss.str() << std::endl;
    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " ERR " << ss.str() << std::endl;
};

void CLog::Err(const void* buf, const size_t size)
{
    std::stringstream ss;

    buf2stream(buf, size, ss);
    Err(ss);
}

void CLog::Detail(const char* message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << message << std::endl;
};

void CLog::Detail(const std::string& message)
{
    std::lock_guard<std::mutex> guard(m_lock);

    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << message << std::endl;
};

void CLog::Detail(const std::stringstream& ss)
{
    std::lock_guard<std::mutex> guard(m_lock);

    if (m_isOpen)
        m_out << std::clock() << " " << std::this_thread::get_id() << " " << ss.str() << std::endl;
};

void CLog::Detail(const void* buf, const size_t size)
{
    std::stringstream ss;

    buf2stream(buf, size, ss);
    Detail(ss);
}

CLog logger;
