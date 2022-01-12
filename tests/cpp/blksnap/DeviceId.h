#include <string>
#include <system_error>
#include <sys/stat.h>

namespace blksnap
{

struct SDeviceId{
    unsigned int mj;
    unsigned int mn;

    SDeviceId()
        : mj(0)
        , mn(0)
    {};

    SDeviceId(int mjr, int mnr)
        : mj(mjr)
        , mn(mnr)
    {};

    std::string ToString() const
    {
        return std::to_string(mj) + ":" + std::to_string(mn);
    };

    bool operator == (const SDeviceId& devId) const
    {
        return (mj == devId.mj) && (mn == devId.mn);
    };

    bool operator != (const SDeviceId& devId) const
    {
        return !operator==(devId);
    };

    bool operator < (const SDeviceId& devId) const
    {
        if (mj < devId.mj)
            return true;

        if (mj == devId.mj)
            if (mn < devId.mn)
                return  true;

        return false;
    };

    static SDeviceId DeviceByName(const std::string &name)
    {
        struct stat st;

        if (::stat(name.c_str(), &st))
            throw std::system_error(errno, std::generic_category(), name);

        return SDeviceId(st.st_rdev, st.st_rdev);
    };
};

}
