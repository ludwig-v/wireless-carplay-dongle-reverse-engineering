/* Reconstructed Carlinkit drivers/crypto/mxs-dcp-hewei.c compatibility. */
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mpi.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <crypto/aes.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>

#define HWAES_ACTIVATE 0x6205
#define HWAES_DECRYPT _IOC(_IOC_READ | _IOC_WRITE, 0x62, 0x06, 0x0c)
#define HWAES_SALTED_MD5 _IOC(_IOC_READ | _IOC_WRITE, 0x62, 0x07, 0x20)
#define HWAES_DECRYPT_BYTES 0x2000
#define RSA_BYTES 256
#define SHA1_BYTES 20
#define OCOTP_PHYS 0x021bc000
#define CCM_PHYS 0x020c4000
#define CCM_CCGR2 0x70
#define CCM_CCGR2_OCOTP_MASK 0x3000

/* i.MX6ULL OCOTP shadow-register offsets used by the vendor activation check. */
#define OCOTP_CFG0 0x410 /* HW_OCOTP_CFG0: customer configuration word 0 */
#define OCOTP_CFG1 0x420 /* HW_OCOTP_CFG1: customer configuration word 1 */
#define OCOTP_MAC0 0x620 /* HW_OCOTP_MAC0: first MAC-address fuse word */
#define OCOTP_MAC1 0x630 /* HW_OCOTP_MAC1: second MAC-address fuse word */

struct hwaes_decrypt32 { u32 input, output, length; };
struct hwaes_activation { u8 signature[RSA_BYTES]; s32 result; };

static void __iomem *ocotp;
static void __iomem *ccm;
static DEFINE_MUTEX(ocotp_lock);
static const u8 aes_key[16] = "coding=utf-8    ";
static const u8 aes_iv[16] = "clang 11.0.0 (cl";
static const u8 md5_salt[16] = "t4FcPZ3qFYphaDa9";

/* RSA-2048 public modulus from the vendor image; exponent is 65537. */
static const u8 rsa_n[RSA_BYTES] = {
	0xa7,0x09,0x43,0xa5,0xca,0xa9,0xab,0x9d,0x83,0x46,0xdf,0x63,0xce,0x82,0x5c,0x9a,
	0x2a,0xf8,0xb9,0xcd,0xc7,0x7c,0x96,0x80,0x54,0xee,0x94,0x5f,0x54,0x0c,0x05,0xaa,
	0x05,0x1c,0x9b,0xa2,0x5c,0x38,0x29,0x05,0x5c,0x37,0x7c,0x74,0x4b,0xae,0xcd,0x73,
	0x51,0x4c,0x3a,0x35,0xbb,0x98,0x22,0x6c,0x22,0x00,0xe6,0xb4,0xf4,0x10,0xd5,0x7f,
	0xda,0x3a,0x89,0x88,0x39,0x9e,0xce,0x31,0xf0,0x04,0x1c,0x13,0x4e,0xf8,0xc1,0x5e,
	0xcf,0x4e,0x0b,0x19,0xc3,0x0e,0x07,0x51,0x15,0x71,0x12,0x9c,0x8e,0x47,0x94,0x1c,
	0x9a,0x38,0xa4,0xf2,0x92,0xb0,0x39,0xf0,0xf5,0xb6,0x43,0xc7,0x3c,0xf7,0x66,0x52,
	0x3a,0xff,0xfa,0xb6,0xfc,0x2c,0x68,0xf5,0x6a,0xf7,0xd2,0x08,0xb8,0x10,0x75,0x5f,
	0xfb,0x02,0xf0,0x3c,0x49,0x62,0x52,0xaf,0x51,0xd7,0xdf,0x71,0xcc,0xfc,0x72,0xae,
	0xa4,0xd0,0x09,0x76,0xca,0x7e,0x1b,0x21,0x68,0xcf,0xd2,0x38,0xfc,0x74,0xcd,0xae,
	0xa8,0x74,0x99,0xfd,0x5b,0x27,0x69,0x92,0x2f,0xd1,0x2d,0xdf,0x03,0x4c,0x77,0x43,
	0xb8,0x3b,0xb3,0x0b,0x02,0x7a,0xcf,0xd6,0x5b,0x7d,0x10,0x0e,0x65,0x4a,0xff,0x22,
	0xbd,0x96,0x77,0xbc,0x45,0x87,0x59,0x73,0x09,0x77,0x3f,0x09,0x3c,0x0f,0x38,0xc3,
	0x47,0x03,0x0a,0x28,0x43,0x90,0x1d,0xa9,0x1c,0x60,0x6b,0x8e,0x1c,0x19,0xd2,0x71,
	0xc6,0x85,0x6a,0xf2,0xea,0x07,0xee,0xba,0xf7,0x2a,0x0c,0xe5,0x37,0x42,0xaa,0x5d,
	0xe3,0x31,0xe7,0x96,0x00,0xb7,0xbb,0xd4,0x5f,0xd8,0xec,0x58,0x53,0x30,0x94,0xfd
};

static int make_digest(const char *algorithm, const void *data,
		       unsigned int len, u8 *digest)
{
	struct crypto_shash *tfm;
	struct shash_desc *desc;
	int ret;

	tfm = crypto_alloc_shash(algorithm, 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);
	desc = kmalloc(sizeof(*desc) + crypto_shash_descsize(tfm), GFP_KERNEL);
	if (!desc) {
		crypto_free_shash(tfm);
		return -ENOMEM;
	}
	desc->tfm = tfm;
	ret = crypto_shash_digest(desc, data, len, digest);
	kfree(desc);
	crypto_free_shash(tfm);
	return ret;
}

static int verify_signature(const u8 *signature, const u8 *digest)
{
	static const u8 rsa_e[] = { 0x01, 0x00, 0x01 };
	MPI s = NULL, n = NULL, e = NULL, m = NULL;
	u8 em[RSA_BYTES], *p = NULL;
	unsigned int len = 0, i;
	int sign = 0, ret = -EKEYREJECTED;

	s = mpi_read_raw_data(signature, RSA_BYTES);
	n = mpi_read_raw_data(rsa_n, RSA_BYTES);
	e = mpi_read_raw_data(rsa_e, sizeof(rsa_e));
	m = mpi_alloc(0);
	if (!s || !n || !e || !m) { ret = -ENOMEM; goto out; }
	if (mpi_cmp(s, n) >= 0 || mpi_powm(m, s, e, n) < 0) goto out;
	p = mpi_get_buffer(m, &len, &sign);
	if (!p || sign || len > sizeof(em)) goto out;
	memset(em, 0, sizeof(em));
	memcpy(em + sizeof(em) - len, p, len);
	if (em[0] || em[1] != 1 || em[RSA_BYTES - SHA1_BYTES - 1]) goto out;
	for (i = 2; i < RSA_BYTES - SHA1_BYTES - 1; i++)
		if (em[i] != 0xff) goto out;
	if (memcmp(em + RSA_BYTES - SHA1_BYTES, digest, SHA1_BYTES)) goto out;
	ret = 0;
out:
	kfree(p); mpi_free(m); mpi_free(e); mpi_free(n); mpi_free(s);
	return ret;
}

static long activate(unsigned long arg)
{
	struct hwaes_activation a;
	char uuid[33];
	u8 digest[SHA1_BYTES];
	u32 ccgr2, cfg0, cfg1, mac0, mac1;
	int ret;

	if (copy_from_user(&a, (void __user *)arg, sizeof(a))) return -EFAULT;
	/*
	 * The vendor calls this value the UUID.  It is the four OCOTP shadow
	 * words concatenated as 32 lowercase hexadecimal characters, without
	 * separators or a trailing NUL in the SHA-1 input:
	 *
	 *     CFG0 || CFG1 || MAC0 || MAC1
	 *
	 * This is also how the userspace /etc/uuid value is produced.
	 */
	/*
	 * The vendor handler gates the OCOTP clock through CCM CCGR2 around
	 * every shadow-register access.  Reading OCOTP while this clock is
	 * gated can stall the ARM bus, which presents as a silent system
	 * lockup rather than a recoverable data abort.
	 */
	mutex_lock(&ocotp_lock);
	ccgr2 = readl(ccm + CCM_CCGR2);
	writel(ccgr2 | CCM_CCGR2_OCOTP_MASK, ccm + CCM_CCGR2);
	readl(ccm + CCM_CCGR2);
	cfg0 = readl(ocotp + OCOTP_CFG0);
	cfg1 = readl(ocotp + OCOTP_CFG1);
	mac0 = readl(ocotp + OCOTP_MAC0);
	mac1 = readl(ocotp + OCOTP_MAC1);
	writel(ccgr2, ccm + CCM_CCGR2);
	readl(ccm + CCM_CCGR2);
	mutex_unlock(&ocotp_lock);

	snprintf(uuid, sizeof(uuid), "%08x%08x%08x%08x",
		 cfg0, cfg1, mac0, mac1);
	ret = make_digest("sha1", uuid, 32, digest);
	if (ret) return ret;
	/*
	 * ioctl 0x6205 copies 0x104 bytes back and writes the activation
	 * result immediately after the 256-byte signature: 1 for a valid
	 * signature and -2 for an invalid/non-activated device.
	 */
	a.result = verify_signature(a.signature, digest) == 0 ? 1 : -2;

	/* Development-only activation bypass: report every device as activated. */
	a.result = 1;
	return copy_to_user((void __user *)arg, &a, sizeof(a)) ? -EFAULT : 0;
}

static long salted_md5(unsigned long arg)
{
	u8 response[32], input[32];
	int ret;

	if (copy_from_user(response, (void __user *)arg, sizeof(response)))
		return -EFAULT;

	/*
	 * Vendor ioctl 0xc0206207 preserves the first 16 input bytes and
	 * replaces the second half with:
	 *
	 *     MD5(input[0:16] || "t4FcPZ3qFYphaDa9")
	 */
	memcpy(input, response, 16);
	memcpy(input + 16, md5_salt, sizeof(md5_salt));
	ret = make_digest("md5", input, sizeof(input), response + 16);
	if (ret)
		return ret;

	return copy_to_user((void __user *)arg, response, sizeof(response)) ?
		-EFAULT : 0;
}

static long decrypt(unsigned long arg)
{
	struct hwaes_decrypt32 p;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req = NULL;
	struct scatterlist src, dst;
	u8 *in, *out = NULL, iv[16];
	void __user *uin, *uout;
	unsigned int offset;
	int ret;

	if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
		return -EFAULT;
	/*
	 * The third word is the total packed-payload length, not the AES
	 * operation length.  The vendor driver first copies the complete
	 * payload from p.input to p.output, then decrypts exactly the first
	 * 0x2000 bytes in place.  Bytes after that encrypted prefix are left
	 * untouched.  hwSecret payloads are larger than 0x2000 and are not
	 * necessarily AES-block aligned.
	 */
	if (p.length < HWAES_DECRYPT_BYTES)
		return -EINVAL;
	uin = (void __user *)(unsigned long)p.input;
	uout = (void __user *)(unsigned long)p.output;
	in = kmalloc(HWAES_DECRYPT_BYTES, GFP_KERNEL);
	if (!in)
		return -ENOMEM;

	/* Copy in bounded chunks instead of dereferencing userspace pointers
	 * directly as the original 3.0-era driver did. */
	for (offset = 0; offset < p.length; ) {
		unsigned int chunk = min_t(unsigned int,
			HWAES_DECRYPT_BYTES, p.length - offset);

		if (copy_from_user(in, (u8 __user *)uin + offset, chunk) ||
		    copy_to_user((u8 __user *)uout + offset, in, chunk)) {
			ret = -EFAULT;
			goto out;
		}
		offset += chunk;
	}

	out = kmalloc(HWAES_DECRYPT_BYTES, GFP_KERNEL);
	if (!out) { ret = -ENOMEM; goto out; }
	tfm = crypto_alloc_skcipher("cbc(aes)", 0, 0);
	if (IS_ERR(tfm)) { ret = PTR_ERR(tfm); goto out; }
	ret = crypto_skcipher_setkey(tfm, aes_key, sizeof(aes_key));
	if (ret) goto out_tfm;
	req = skcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) { ret = -ENOMEM; goto out_tfm; }
	if (copy_from_user(in, uout, HWAES_DECRYPT_BYTES)) {
		ret = -EFAULT;
		goto out_tfm;
	}
	memcpy(iv, aes_iv, sizeof(iv));
	sg_init_one(&src, in, HWAES_DECRYPT_BYTES);
	sg_init_one(&dst, out, HWAES_DECRYPT_BYTES);
	skcipher_request_set_crypt(req, &src, &dst, HWAES_DECRYPT_BYTES, iv);
	ret = crypto_skcipher_decrypt(req);
	if (!ret && copy_to_user(uout, out, HWAES_DECRYPT_BYTES))
		ret = -EFAULT;
out_tfm:
	skcipher_request_free(req);
	crypto_free_skcipher(tfm);
out:
	kfree(out); kfree(in); return ret;
}

static long hwaes_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	if (cmd == HWAES_ACTIVATE) return activate(arg);
	if (cmd == HWAES_DECRYPT) return decrypt(arg);
	if (cmd == HWAES_SALTED_MD5) return salted_md5(arg);
	return -ENOTTY;
}

static const struct file_operations hwaes_fops = {
	.owner = THIS_MODULE, .unlocked_ioctl = hwaes_ioctl,
};
static struct miscdevice hwaes_dev = {
	.minor = MISC_DYNAMIC_MINOR, .name = "hwaes", .fops = &hwaes_fops,
};

static int __init hwaes_init(void)
{
	int ret;
	ocotp = ioremap(OCOTP_PHYS, 0x1000);
	if (!ocotp) return -ENOMEM;
	ccm = ioremap(CCM_PHYS, 0x1000);
	if (!ccm) {
		iounmap(ocotp);
		ocotp = NULL;
		return -ENOMEM;
	}
	ret = misc_register(&hwaes_dev);
	if (ret) {
		iounmap(ccm); ccm = NULL;
		iounmap(ocotp); ocotp = NULL;
	}
	return ret;
}
static void __exit hwaes_exit(void)
{
	misc_deregister(&hwaes_dev);
	if (ccm) iounmap(ccm);
	if (ocotp) iounmap(ocotp);
}
module_init(hwaes_init); module_exit(hwaes_exit);
MODULE_DESCRIPTION("Carlinkit MXS DCP Hewei /dev/hwaes compatibility driver");
MODULE_AUTHOR("Carlinkit kernel reconstruction project");
MODULE_LICENSE("GPL");
