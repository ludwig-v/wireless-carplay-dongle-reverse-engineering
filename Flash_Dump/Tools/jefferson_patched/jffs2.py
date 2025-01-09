import binascii
import contextlib
import mmap
import os
import stat
import struct
import sys
import zlib
from pathlib import Path

import cstruct
from lzallright import LZOCompressor as lzo

import jefferson.compression.jffs2_lzma as jffs2_lzma
import jefferson.compression.rtime as rtime


def PAD(x):
    return ((x) + 3) & ~3


JFFS2_OLD_MAGIC_BITMASK = 0x1984
JFFS2_MAGIC_BITMASK = 0x1985
JFFS2_COMPR_NONE = 0x00
JFFS2_COMPR_ZERO = 0x01
JFFS2_COMPR_RTIME = 0x02
JFFS2_COMPR_RUBINMIPS = 0x03
JFFS2_COMPR_COPY = 0x04
JFFS2_COMPR_DYNRUBIN = 0x05
JFFS2_COMPR_ZLIB = 0x06
JFFS2_COMPR_LZO = 0x07
JFFS2_COMPR_LZMA = 0x08
JFFS2_COMPR_CARLINKIT_ZLIB = 0x16

# /* Compatibility flags. */
JFFS2_COMPAT_MASK = 0xC000  # /* What do to if an unknown nodetype is found */
JFFS2_NODE_ACCURATE = 0x2000
# /* INCOMPAT: Fail to mount the filesystem */
JFFS2_FEATURE_INCOMPAT = 0xC000
# /* ROCOMPAT: Mount read-only */
JFFS2_FEATURE_ROCOMPAT = 0x8000
# /* RWCOMPAT_COPY: Mount read/write, and copy the node when it's GC'd */
JFFS2_FEATURE_RWCOMPAT_COPY = 0x4000
# /* RWCOMPAT_DELETE: Mount read/write, and delete the node when it's GC'd */
JFFS2_FEATURE_RWCOMPAT_DELETE = 0x0000

JFFS2_NODETYPE_DIRENT = JFFS2_FEATURE_INCOMPAT | JFFS2_NODE_ACCURATE | 1
JFFS2_NODETYPE_INODE = JFFS2_FEATURE_INCOMPAT | JFFS2_NODE_ACCURATE | 2
JFFS2_NODETYPE_CLEANMARKER = JFFS2_FEATURE_RWCOMPAT_DELETE | JFFS2_NODE_ACCURATE | 3
JFFS2_NODETYPE_PADDING = JFFS2_FEATURE_RWCOMPAT_DELETE | JFFS2_NODE_ACCURATE | 4
JFFS2_NODETYPE_SUMMARY = JFFS2_FEATURE_RWCOMPAT_DELETE | JFFS2_NODE_ACCURATE | 6
JFFS2_NODETYPE_XATTR = JFFS2_FEATURE_INCOMPAT | JFFS2_NODE_ACCURATE | 8
JFFS2_NODETYPE_XREF = JFFS2_FEATURE_INCOMPAT | JFFS2_NODE_ACCURATE | 9


def mtd_crc(data):
    return (binascii.crc32(data, -1) ^ -1) & 0xFFFFFFFF


def is_safe_path(basedir, real_path):
    basedir = os.path.realpath(basedir)
    return basedir == os.path.commonpath((basedir, real_path))


cstruct.typedef("uint8", "uint8_t")
cstruct.typedef("uint16", "jint16_t")
cstruct.typedef("uint32", "jint32_t")
cstruct.typedef("uint32", "jmode_t")


class Jffs2_unknown_node(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __def__ = """
        struct {
            /* All start like this */
            jint16_t magic;
            jint16_t nodetype;
            jint32_t totlen; /* So we can skip over nodes we don't grok */
            jint32_t hdr_crc;
        }
    """

    def unpack(self, data):
        cstruct.CStruct.unpack(self, data[: self.size])
        comp_hrd_crc = mtd_crc(data[: self.size - 4])

        if comp_hrd_crc == self.hdr_crc:
            self.hdr_crc_match = True
        else:
            # print("hdr_crc does not match!")
            self.hdr_crc_match = False


class Jffs2_raw_dirent(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __def__ = """
        struct {
            jint16_t magic;
            jint16_t nodetype;      /* == JFFS2_NODETYPE_DIRENT */
            jint32_t totlen;
            jint32_t hdr_crc;
            jint32_t pino;
            jint32_t version;
            jint32_t ino; /* == zero for unlink */
            jint32_t mctime;
            uint8_t nsize;
            uint8_t type;
            uint8_t unused[2];
            jint32_t node_crc;
            jint32_t name_crc;
        /* uint8_t data[0]; -> name */
        }
    """

    def unpack(self, data, node_offset):
        cstruct.CStruct.unpack(self, data[: self.size])
        self.name = data[self.size : self.size + self.nsize].tobytes()
        self.node_offset = node_offset

        if mtd_crc(data[: self.size - 8]) == self.node_crc:
            self.node_crc_match = True
        else:
            print("node_crc does not match!")
            self.node_crc_match = False

        if mtd_crc(self.name) == self.name_crc:
            self.name_crc_match = True
        else:
            print("data_crc does not match!")
            self.name_crc_match = False

    def __str__(self):
        result = []
        for field in self.__fields__ + ["name", "node_offset"]:
            result.append(field + "=" + str(getattr(self, field, None)))
        return type(self).__name__ + "(" + ", ".join(result) + ")"


class Jffs2_raw_inode(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __def__ = """
        struct {
            jint16_t magic;      /* A constant magic number.  */
            jint16_t nodetype;   /* == JFFS2_NODETYPE_INODE */
            jint32_t totlen;     /* Total length of this node (inc data, etc.) */
            jint32_t hdr_crc;
            jint32_t ino;        /* Inode number.  */
            jint32_t version;    /* Version number.  */
            jmode_t mode;       /* The file's type or mode.  */
            jint16_t uid;        /* The file's owner.  */
            jint16_t gid;        /* The file's group.  */
            jint32_t isize;      /* Total resultant size of this inode (used for truncations)  */
            jint32_t atime;      /* Last access time.  */
            jint32_t mtime;      /* Last modification time.  */
            jint32_t ctime;      /* Change time.  */
            jint32_t offset;     /* Where to begin to write.  */
            jint32_t csize;      /* (Compressed) data size */
            jint32_t dsize;      /* Size of the node's data. (after decompression) */
            uint8_t compr;       /* Compression algorithm used */
            uint8_t usercompr;   /* Compression algorithm requested by the user */
            jint16_t flags;      /* See JFFS2_INO_FLAG_* */
            jint32_t data_crc;   /* CRC for the (compressed) data.  */
            jint32_t node_crc;   /* CRC for the raw inode (excluding data)  */
            /* uint8_t data[0]; */
        }
    """

    def unpack(self, data):
        cstruct.CStruct.unpack(self, data[: self.size])

        node_data = data[self.size : self.size + self.csize].tobytes()
        try:
            if self.compr == JFFS2_COMPR_NONE:
                self.data = node_data
            elif self.compr == JFFS2_COMPR_CARLINKIT_ZLIB:
                swapped_data = bytearray(node_data)
                for i in range(len(swapped_data)):
                    if swapped_data[i] == 0x32:
                        swapped_data[i] = 0x60
                    elif swapped_data[i] == 0x60:
                       swapped_data[i] = 0x32
                self.data = zlib.decompress(bytes(swapped_data))
            elif self.compr == JFFS2_COMPR_ZERO:
                self.data = b"\x00" * self.dsize
            elif self.compr == JFFS2_COMPR_ZLIB:
                self.data = zlib.decompress(node_data)
            elif self.compr == JFFS2_COMPR_RTIME:
                self.data = rtime.decompress(node_data, self.dsize)
            elif self.compr == JFFS2_COMPR_LZMA:
                self.data = jffs2_lzma.decompress(node_data, self.dsize)
            elif self.compr == JFFS2_COMPR_LZO:
                self.data = lzo.decompress(node_data)
            else:
                print("compression not implemented", self)
                print(node_data.hex()[:20])
                self.data = node_data
        except Exception as e:
            print(
                "Original data written due to decompression error on inode {}: {}".format(self.ino, e),
                file=sys.stderr,
            )
            self.data = node_data

        if len(self.data) != self.dsize:
            print("data length mismatch!")

        if mtd_crc(data[: self.size - 8]) == self.node_crc:
            self.node_crc_match = True
        else:
            print("hdr_crc does not match!")
            self.node_crc_match = False

        if mtd_crc(node_data) == self.data_crc:
            self.data_crc_match = True
        else:
            print("data_crc does not match!")
            self.data_crc_match = False


class Jffs2_device_node_old(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __def__ = """
        struct {
            jint16_t old_id;
        }
    """


class Jffs2_device_node_new(cstruct.CStruct):
    __byte_order__ = cstruct.LITTLE_ENDIAN
    __def__ = """
        struct {
            jint32_t new_id;
        }
    """


NODETYPES = {
    JFFS2_FEATURE_INCOMPAT: Jffs2_unknown_node,
    JFFS2_NODETYPE_DIRENT: Jffs2_raw_dirent,
    JFFS2_NODETYPE_INODE: Jffs2_raw_inode,
    JFFS2_NODETYPE_CLEANMARKER: "JFFS2_NODETYPE_CLEANMARKER",
    JFFS2_NODETYPE_PADDING: "JFFS2_NODETYPE_PADDING",
}


def set_endianness(endianness):
    global Jffs2_device_node_new, Jffs2_device_node_old, Jffs2_unknown_node, Jffs2_raw_dirent, Jffs2_raw_inode, Jffs2_raw_summary, Jffs2_raw_xattr, Jffs2_raw_xref

    Jffs2_device_node_new = Jffs2_device_node_new.parse(
        Jffs2_device_node_new.__def__,
        __name__=Jffs2_device_node_new.__name__,
        __byte_order__=endianness,
    )

    Jffs2_device_node_old = Jffs2_device_node_old.parse(
        Jffs2_device_node_old.__def__,
        __name__=Jffs2_device_node_old.__name__,
        __byte_order__=endianness,
    )

    Jffs2_unknown_node = Jffs2_unknown_node.parse(
        Jffs2_unknown_node.__def__,
        __name__=Jffs2_unknown_node.__name__,
        __byte_order__=endianness,
    )

    Jffs2_raw_dirent = Jffs2_raw_dirent.parse(
        Jffs2_raw_dirent.__def__,
        __name__=Jffs2_raw_dirent.__name__,
        __byte_order__=endianness,
    )

    Jffs2_raw_inode = Jffs2_raw_inode.parse(
        Jffs2_raw_inode.__def__,
        __name__=Jffs2_raw_inode.__name__,
        __byte_order__=endianness,
    )


def scan_fs(content, endianness, verbose=False):
    pos = 0
    jffs2_old_magic_bitmask_str = struct.pack(endianness + "H", JFFS2_OLD_MAGIC_BITMASK)
    jffs2_magic_bitmask_str = struct.pack(endianness + "H", JFFS2_MAGIC_BITMASK)
    content_mv = memoryview(content)

    fs = {}
    fs[JFFS2_NODETYPE_INODE] = {}
    fs[JFFS2_NODETYPE_DIRENT] = {}

    while True:
        find_result = content.find(
            jffs2_magic_bitmask_str, pos, len(content) - Jffs2_unknown_node.size
        )
        find_result_old = content.find(
            jffs2_old_magic_bitmask_str, pos, len(content) - Jffs2_unknown_node.size
        )
        if find_result == -1 and find_result_old == -1:
            break
        if find_result != -1:
            pos = find_result
        else:
            pos = find_result_old

        unknown_node = Jffs2_unknown_node()
        unknown_node.unpack(content_mv[pos : pos + unknown_node.size])
        if not unknown_node.hdr_crc_match:
            pos += 1
            continue
        offset = pos
        pos += PAD(unknown_node.totlen)

        if unknown_node.magic in [
            JFFS2_MAGIC_BITMASK,
            JFFS2_OLD_MAGIC_BITMASK,
        ]:
            if unknown_node.nodetype in NODETYPES:
                if unknown_node.nodetype == JFFS2_NODETYPE_DIRENT:
                    dirent = Jffs2_raw_dirent()
                    dirent.unpack(content_mv[0 + offset :], offset)
                    if dirent.ino in fs[JFFS2_NODETYPE_DIRENT]:
                        if (
                            dirent.version
                            > fs[JFFS2_NODETYPE_DIRENT][dirent.ino].version
                        ):
                            fs[JFFS2_NODETYPE_DIRENT][dirent.ino] = dirent
                    else:
                        fs[JFFS2_NODETYPE_DIRENT][dirent.ino] = dirent
                    if verbose:
                        print("0x%08X:" % (offset), dirent)
                elif unknown_node.nodetype == JFFS2_NODETYPE_INODE:
                    inode = Jffs2_raw_inode()
                    inode.unpack(content_mv[0 + offset :])

                    if inode.ino in fs[JFFS2_NODETYPE_INODE]:
                        fs[JFFS2_NODETYPE_INODE][inode.ino].append(inode)
                    else:
                        fs[JFFS2_NODETYPE_INODE][inode.ino] = [inode]
                    if verbose:
                        print("0x%08X:" % (offset), inode)
                elif unknown_node.nodetype == JFFS2_NODETYPE_CLEANMARKER:
                    pass
                elif unknown_node.nodetype == JFFS2_NODETYPE_PADDING:
                    pass
                elif unknown_node.nodetype == JFFS2_NODETYPE_SUMMARY:
                    pass
                elif unknown_node.nodetype == JFFS2_NODETYPE_XATTR:
                    pass
                elif unknown_node.nodetype == JFFS2_NODETYPE_XREF:
                    pass
                else:
                    print("Unknown node type", unknown_node.nodetype, unknown_node)
    content_mv.release()
    return fs


def get_device(inode):
    if not stat.S_ISBLK(inode.mode) and not stat.S_ISCHR(inode.mode):
        return None

    if inode.dsize == len(Jffs2_device_node_new):
        node = Jffs2_device_node_new()
        node.unpack(inode.data)
        return os.makedev(
            (node.new_id & 0xFFF00) >> 8,
            (node.new_id & 0xFF) | ((node.new_id >> 12) & 0xFFF00),
        )

    if inode.dsize == len(Jffs2_device_node_old):
        node = Jffs2_device_node_old()
        node.unpack(inode.data)
        return os.makedev((node.old_id >> 8) & 0xFF, node.old_id & 0xFF)
    return None


def sort_version(item):
    return item.version


def dump_fs(fs, target):
    node_dict = {}

    for dirent in fs[JFFS2_NODETYPE_DIRENT].values():
        dirent.inodes = []
        for ino, inodes in fs[JFFS2_NODETYPE_INODE].items():
            if ino == dirent.ino:
                dirent.inodes = sorted(inodes, key=sort_version)
        node_dict[dirent.ino] = dirent

    for dirent in fs[JFFS2_NODETYPE_DIRENT].values():
        pnode_pino = dirent.pino
        pnodes = []
        for _ in range(100):
            if pnode_pino not in node_dict:
                break
            pnode = node_dict[pnode_pino]
            pnode_pino = pnode.pino
            pnodes.append(pnode)
        pnodes.reverse()

        node_names = []

        for pnode in pnodes:
            node_names.append(pnode.name.decode())
        node_names.append(dirent.name.decode())
        path = "/".join(node_names)

        target_path = os.path.realpath(os.path.join(target, path))

        if not is_safe_path(target, target_path):
            print(f"Path traversal attempt to {target_path}, discarding.")
            continue

        for inode in dirent.inodes:
            try:
                if stat.S_ISDIR(inode.mode):
                    print("writing S_ISDIR", path)
                    if not os.path.isdir(target_path):
                        os.makedirs(target_path)
                elif stat.S_ISLNK(inode.mode):
                    print("writing S_ISLNK", path)
                    if not os.path.isdir(os.path.dirname(target_path)):
                       os.makedirs(os.path.dirname(target_path))
                    if not os.path.islink(target_path):
                        if os.path.exists(target_path):
                            continue
                        os.symlink(inode.data, target_path)
                elif stat.S_ISREG(inode.mode):
                    print("writing S_ISREG", path)
                    if not os.path.isfile(target_path):
                        if not os.path.isdir(os.path.dirname(target_path)):
                            os.makedirs(os.path.dirname(target_path))
                        with open(target_path, "wb") as fd:
                            for inode in dirent.inodes:
                                fd.seek(inode.offset)
                                fd.write(inode.data)
                    os.chmod(target_path, stat.S_IMODE(inode.mode))
                    break
                elif stat.S_ISCHR(inode.mode):
                    print("writing S_ISBLK", path)
                    os.mknod(target_path, inode.mode, get_device(inode))
                elif stat.S_ISBLK(inode.mode):
                    print("writing S_ISBLK", path)
                    os.mknod(target_path, inode.mode, get_device(inode))
                elif stat.S_ISFIFO(inode.mode):
                    print("skipping S_ISFIFO", path)
                elif stat.S_ISSOCK(inode.mode):
                    print("skipping S_ISSOCK", path)
                else:
                    print("unhandled inode.mode: %o" % inode.mode, inode, dirent)

            except OSError as error:
                print("OS error(%i): %s" % (error.errno, error.strerror), inode, dirent)


def extract_jffs2(file: Path, destination: Path, verbose: int) -> int:
    with contextlib.ExitStack() as context_stack:
        filesystem = context_stack.enter_context(file.open("rb"))
        filesystem_len = os.fstat(filesystem.fileno()).st_size
        if 0 == filesystem_len:
            return -1
        content = context_stack.enter_context(
            mmap.mmap(filesystem.fileno(), filesystem_len, access=mmap.ACCESS_READ)
        )
        magic = struct.unpack("<H", content[0:2])[0]
        if magic in [JFFS2_OLD_MAGIC_BITMASK, JFFS2_MAGIC_BITMASK]:
            endianness = cstruct.LITTLE_ENDIAN
        else:
            endianness = cstruct.BIG_ENDIAN

        set_endianness(endianness)

        fs = scan_fs(content, endianness, verbose=verbose)
        print("dumping fs to %s (endianness: %s)" % (destination, endianness))
        for key, value in fs.items():
            print("%s count: %i" % (NODETYPES[key].__name__, len(value)))

        dump_fs(fs, destination)
    return 0
