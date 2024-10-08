#include "lib/core/globals.h"
#include "lib/flags.h"
#include "lib/proto.h"
#include "fluxengine.h"
#include "lib/vfs/vfs.h"
#include "lib/core/utils.h"
#include "src/fileutils.h"
#include <google/protobuf/text_format.h>
#include <fstream>

static FlagGroup flags({&fileFlags});

static StringFlag filename({"-p", "--path"}, "directory to create", "");

int mainMkDir(int argc, const char* argv[])
{
    if (argc == 1)
        showProfiles("mkdir", formats);
    flags.parseFlagsWithConfigFiles(argc, argv, formats);

    try
    {
        auto filesystem = Filesystem::createFilesystemFromConfig();

        Path path(filename);
        if (path.size() == 0)
            error("filename missing");

        filesystem->createDirectory(path);
        filesystem->flushChanges();
    }
    catch (const FilesystemException& e)
    {
        error("{}", e.message);
    }

    return 0;
}
