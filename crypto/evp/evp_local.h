/*
 * Copyright 2000-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* EVP_MD_CTX related stuff */

#include <openssl/core_dispatch.h>
#include "internal/refcount.h"

#define EVP_CTRL_RET_UNSUPPORTED -1


struct evp_md_ctx_st {
    const EVP_MD *reqdigest;    /* The original requested digest */
    const EVP_MD *digest;
    ENGINE *engine;             /* functional reference if 'digest' is
                                 * ENGINE-provided */
    unsigned long flags;
    void *md_data;
    /* Public key context for sign/verify */
    EVP_PKEY_CTX *pctx;
    /* Update function: usually copied from EVP_MD */
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);

    /* Provider ctx */
    void *provctx;
    EVP_MD *fetched_digest;
} /* EVP_MD_CTX */ ;

struct evp_cipher_ctx_st {
    const EVP_CIPHER *cipher;
    ENGINE *engine;             /* functional reference if 'cipher' is
                                 * ENGINE-provided */
    int encrypt;                /* encrypt or decrypt */
    int buf_len;                /* number we have left */
    unsigned char oiv[EVP_MAX_IV_LENGTH]; /* original iv */
    unsigned char iv[EVP_MAX_IV_LENGTH]; /* working iv */
    unsigned char buf[EVP_MAX_BLOCK_LENGTH]; /* saved partial block */
    int num;                    /* used by cfb/ofb/ctr mode */
    /* FIXME: Should this even exist? It appears unused */
    void *app_data;             /* application stuff */
    int key_len;                /* May change for variable length cipher */
    unsigned long flags;        /* Various flags */
    void *cipher_data;          /* per EVP data */
    int final_used;
    int block_mask;
    unsigned char final[EVP_MAX_BLOCK_LENGTH]; /* possible final block */

    /* Provider ctx */
    void *provctx;
    EVP_CIPHER *fetched_cipher;
} /* EVP_CIPHER_CTX */ ;

struct evp_mac_ctx_st {
    EVP_MAC *meth;               /* Method structure */
    void *data;                  /* Individual method data */
} /* EVP_MAC_CTX */;

struct evp_kdf_ctx_st {
    EVP_KDF *meth;              /* Method structure */
    void *data;                 /* Algorithm-specific data */
} /* EVP_KDF_CTX */ ;

struct evp_rand_ctx_st {
    EVP_RAND *meth;             /* Method structure */
    void *data;                 /* Algorithm-specific data */
} /* EVP_RAND_CTX */ ;

struct evp_rand_st {
    OSSL_PROVIDER *prov;
    int name_id;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *refcnt_lock;

    const OSSL_DISPATCH *dispatch;
    OSSL_FUNC_rand_newctx_fn *newctx;
    OSSL_FUNC_rand_freectx_fn *freectx;
    OSSL_FUNC_rand_instantiate_fn *instantiate;
    OSSL_FUNC_rand_uninstantiate_fn *uninstantiate;
    OSSL_FUNC_rand_generate_fn *generate;
    OSSL_FUNC_rand_reseed_fn *reseed;
    OSSL_FUNC_rand_nonce_fn *nonce;
    OSSL_FUNC_rand_enable_locking_fn *enable_locking;
    OSSL_FUNC_rand_lock_fn *lock;
    OSSL_FUNC_rand_unlock_fn *unlock;
    OSSL_FUNC_rand_gettable_params_fn *gettable_params;
    OSSL_FUNC_rand_gettable_ctx_params_fn *gettable_ctx_params;
    OSSL_FUNC_rand_settable_ctx_params_fn *settable_ctx_params;
    OSSL_FUNC_rand_get_params_fn *get_params;
    OSSL_FUNC_rand_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_rand_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_rand_set_callbacks_fn *set_callbacks;
    OSSL_FUNC_rand_verify_zeroization_fn *verify_zeroization;
} /* EVP_RAND */ ;

struct evp_keymgmt_st {
    int id;                      /* libcrypto internal */

    int name_id;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;

    /* Constructor(s), destructor, information */
    OSSL_FUNC_keymgmt_new_fn *new;
    OSSL_FUNC_keymgmt_free_fn *free;
    OSSL_FUNC_keymgmt_get_params_fn *get_params;
    OSSL_FUNC_keymgmt_gettable_params_fn *gettable_params;
    OSSL_FUNC_keymgmt_set_params_fn *set_params;
    OSSL_FUNC_keymgmt_settable_params_fn *settable_params;

    /* Generation, a complex constructor */
    OSSL_FUNC_keymgmt_gen_init_fn *gen_init;
    OSSL_FUNC_keymgmt_gen_set_template_fn *gen_set_template;
    OSSL_FUNC_keymgmt_gen_set_params_fn *gen_set_params;
    OSSL_FUNC_keymgmt_gen_settable_params_fn *gen_settable_params;
    OSSL_FUNC_keymgmt_gen_fn *gen;
    OSSL_FUNC_keymgmt_gen_cleanup_fn *gen_cleanup;

    OSSL_FUNC_keymgmt_load_fn *load;

    /* Key object checking */
    OSSL_FUNC_keymgmt_query_operation_name_fn *query_operation_name;
    OSSL_FUNC_keymgmt_has_fn *has;
    OSSL_FUNC_keymgmt_validate_fn *validate;
    OSSL_FUNC_keymgmt_match_fn *match;

    /* Import and export routines */
    OSSL_FUNC_keymgmt_import_fn *import;
    OSSL_FUNC_keymgmt_import_types_fn *import_types;
    OSSL_FUNC_keymgmt_export_fn *export;
    OSSL_FUNC_keymgmt_export_types_fn *export_types;
    OSSL_FUNC_keymgmt_copy_fn *copy;
} /* EVP_KEYMGMT */ ;

struct evp_keyexch_st {
    int name_id;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;

    OSSL_FUNC_keyexch_newctx_fn *newctx;
    OSSL_FUNC_keyexch_init_fn *init;
    OSSL_FUNC_keyexch_set_peer_fn *set_peer;
    OSSL_FUNC_keyexch_derive_fn *derive;
    OSSL_FUNC_keyexch_freectx_fn *freectx;
    OSSL_FUNC_keyexch_dupctx_fn *dupctx;
    OSSL_FUNC_keyexch_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_keyexch_settable_ctx_params_fn *settable_ctx_params;
    OSSL_FUNC_keyexch_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_keyexch_gettable_ctx_params_fn *gettable_ctx_params;
} /* EVP_KEYEXCH */;

struct evp_signature_st {
    int name_id;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;

    OSSL_FUNC_signature_newctx_fn *newctx;
    OSSL_FUNC_signature_sign_init_fn *sign_init;
    OSSL_FUNC_signature_sign_fn *sign;
    OSSL_FUNC_signature_verify_init_fn *verify_init;
    OSSL_FUNC_signature_verify_fn *verify;
    OSSL_FUNC_signature_verify_recover_init_fn *verify_recover_init;
    OSSL_FUNC_signature_verify_recover_fn *verify_recover;
    OSSL_FUNC_signature_digest_sign_init_fn *digest_sign_init;
    OSSL_FUNC_signature_digest_sign_update_fn *digest_sign_update;
    OSSL_FUNC_signature_digest_sign_final_fn *digest_sign_final;
    OSSL_FUNC_signature_digest_sign_fn *digest_sign;
    OSSL_FUNC_signature_digest_verify_init_fn *digest_verify_init;
    OSSL_FUNC_signature_digest_verify_update_fn *digest_verify_update;
    OSSL_FUNC_signature_digest_verify_final_fn *digest_verify_final;
    OSSL_FUNC_signature_digest_verify_fn *digest_verify;
    OSSL_FUNC_signature_freectx_fn *freectx;
    OSSL_FUNC_signature_dupctx_fn *dupctx;
    OSSL_FUNC_signature_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_signature_gettable_ctx_params_fn *gettable_ctx_params;
    OSSL_FUNC_signature_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_signature_settable_ctx_params_fn *settable_ctx_params;
    OSSL_FUNC_signature_get_ctx_md_params_fn *get_ctx_md_params;
    OSSL_FUNC_signature_gettable_ctx_md_params_fn *gettable_ctx_md_params;
    OSSL_FUNC_signature_set_ctx_md_params_fn *set_ctx_md_params;
    OSSL_FUNC_signature_settable_ctx_md_params_fn *settable_ctx_md_params;
} /* EVP_SIGNATURE */;

struct evp_asym_cipher_st {
    int name_id;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;

    OSSL_FUNC_asym_cipher_newctx_fn *newctx;
    OSSL_FUNC_asym_cipher_encrypt_init_fn *encrypt_init;
    OSSL_FUNC_asym_cipher_encrypt_fn *encrypt;
    OSSL_FUNC_asym_cipher_decrypt_init_fn *decrypt_init;
    OSSL_FUNC_asym_cipher_decrypt_fn *decrypt;
    OSSL_FUNC_asym_cipher_freectx_fn *freectx;
    OSSL_FUNC_asym_cipher_dupctx_fn *dupctx;
    OSSL_FUNC_asym_cipher_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_asym_cipher_gettable_ctx_params_fn *gettable_ctx_params;
    OSSL_FUNC_asym_cipher_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_asym_cipher_settable_ctx_params_fn *settable_ctx_params;
} /* EVP_ASYM_CIPHER */;

int PKCS5_v2_PBKDF2_keyivgen(EVP_CIPHER_CTX *ctx, const char *pass,
                             int passlen, ASN1_TYPE *param,
                             const EVP_CIPHER *c, const EVP_MD *md,
                             int en_de);

struct evp_Encode_Ctx_st {
    /* number saved in a partial encode/decode */
    int num;
    /*
     * The length is either the output line length (in input bytes) or the
     * shortest input line length that is ok.  Once decoding begins, the
     * length is adjusted up each time a longer line is decoded
     */
    int length;
    /* data to encode */
    unsigned char enc_data[80];
    /* number read on current line */
    int line_num;
    unsigned int flags;
};

typedef struct evp_pbe_st EVP_PBE_CTL;
DEFINE_STACK_OF(EVP_PBE_CTL)

int is_partially_overlapping(const void *ptr1, const void *ptr2, int len);

#include <openssl/types.h>
#include <openssl/core.h>

void *evp_generic_fetch(OPENSSL_CTX *ctx, int operation_id,
                        const char *name, const char *properties,
                        void *(*new_method)(int name_id,
                                            const OSSL_DISPATCH *fns,
                                            OSSL_PROVIDER *prov),
                        int (*up_ref_method)(void *),
                        void (*free_method)(void *));
void *evp_generic_fetch_by_number(OPENSSL_CTX *ctx, int operation_id,
                                  int name_id, const char *properties,
                                  void *(*new_method)(int name_id,
                                                      const OSSL_DISPATCH *fns,
                                                      OSSL_PROVIDER *prov),
                                  int (*up_ref_method)(void *),
                                  void (*free_method)(void *));
void evp_generic_do_all(OPENSSL_CTX *libctx, int operation_id,
                        void (*user_fn)(void *method, void *arg),
                        void *user_arg,
                        void *(*new_method)(int name_id,
                                            const OSSL_DISPATCH *fns,
                                            OSSL_PROVIDER *prov),
                        void (*free_method)(void *));

/* Internal fetchers for method types that are to be combined with others */
EVP_KEYMGMT *evp_keymgmt_fetch_by_number(OPENSSL_CTX *ctx, int name_id,
                                         const char *properties);

/* Internal structure constructors for fetched methods */
EVP_MD *evp_md_new(void);
EVP_CIPHER *evp_cipher_new(void);

/* Helper functions to avoid duplicating code */

/*
 * These methods implement different ways to pass a params array to the
 * provider.  They will return one of these values:
 *
 * -2 if the method doesn't come from a provider
 *    (evp_do_param will return this to the called)
 * -1 if the provider doesn't offer the desired function
 *    (evp_do_param will raise an error and return 0)
 * or the return value from the desired function
 *    (evp_do_param will return it to the caller)
 */
int evp_do_ciph_getparams(const EVP_CIPHER *ciph, OSSL_PARAM params[]);
int evp_do_ciph_ctx_getparams(const EVP_CIPHER *ciph, void *provctx,
                              OSSL_PARAM params[]);
int evp_do_ciph_ctx_setparams(const EVP_CIPHER *ciph, void *provctx,
                              OSSL_PARAM params[]);
int evp_do_md_getparams(const EVP_MD *md, OSSL_PARAM params[]);
int evp_do_md_ctx_getparams(const EVP_MD *md, void *provctx,
                            OSSL_PARAM params[]);
int evp_do_md_ctx_setparams(const EVP_MD *md, void *provctx,
                            OSSL_PARAM params[]);

OSSL_PARAM *evp_pkey_to_param(EVP_PKEY *pkey, size_t *sz);

#define M_check_autoarg(ctx, arg, arglen, err) \
    if (ctx->pmeth->flags & EVP_PKEY_FLAG_AUTOARGLEN) {           \
        size_t pksize = (size_t)EVP_PKEY_size(ctx->pkey);         \
                                                                  \
        if (pksize == 0) {                                        \
            ERR_raise(ERR_LIB_EVP, EVP_R_INVALID_KEY); /*ckerr_ignore*/ \
            return 0;                                             \
        }                                                         \
        if (arg == NULL) {                                        \
            *arglen = pksize;                                     \
            return 1;                                             \
        }                                                         \
        if (*arglen < pksize) {                                   \
            ERR_raise(ERR_LIB_EVP, EVP_R_BUFFER_TOO_SMALL); /*ckerr_ignore*/ \
            return 0;                                             \
        }                                                         \
    }

void evp_pkey_ctx_free_old_ops(EVP_PKEY_CTX *ctx);

/* OSSL_PROVIDER * is only used to get the library context */
const char *evp_first_name(const OSSL_PROVIDER *prov, int name_id);
int evp_is_a(OSSL_PROVIDER *prov, int number,
             const char *legacy_name, const char *name);
void evp_names_do_all(OSSL_PROVIDER *prov, int number,
                      void (*fn)(const char *name, void *data),
                      void *data);
int evp_cipher_cache_constants(EVP_CIPHER *cipher);
