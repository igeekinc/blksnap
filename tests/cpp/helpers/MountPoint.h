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
#pragma once

#include <boost/filesystem.hpp>

class MountPoint
{
public:
    MountPoint(boost::filesystem::path device, boost::filesystem::path mountPoint);
    ~MountPoint();

    boost::filesystem::path GetDevice() const;
    boost::filesystem::path GetMountPoint() const;

private:
    void Mount();
    void UnMount();

private:
    boost::filesystem::path m_device;
    boost::filesystem::path m_mountPoint;
};
