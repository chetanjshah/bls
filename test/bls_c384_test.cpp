#include <cybozu/test.hpp>
#include <cybozu/inttype.hpp>
#include <bls/bls.h>
#include <string.h>

void bls_use_stackTest()
{
	blsSecretKey sec;
	blsPublicKey pub;
	blsSignature sig;
	const char *msg = "this is a pen";
	const size_t msgSize = strlen(msg);

	blsSecretKeySetByCSPRNG(&sec);

	blsGetPublicKey(&pub, &sec);

	blsSign(&sig, &sec, msg, msgSize);

	CYBOZU_TEST_ASSERT(blsVerify(&sig, &pub, msg, msgSize));
}

void blsDataTest()
{
	const char *msg = "test test";
	const size_t msgSize = strlen(msg);
	const size_t fpSize = blsGetOpUnitSize() * sizeof(uint64_t);
	blsSecretKey sec1, sec2;
	blsSecretKeySetByCSPRNG(&sec1);
	char buf[1024];
	size_t n;
	int ret;
	n = blsSecretKeyGetHexStr(buf, sizeof(buf), &sec1);
	CYBOZU_TEST_ASSERT(0 < n && n <= fpSize * 2);
	ret = blsSecretKeySetHexStr(&sec2, buf, n);
	CYBOZU_TEST_EQUAL(ret, 0);
	CYBOZU_TEST_ASSERT(blsSecretKeyIsEqual(&sec1, &sec2));

	memset(&sec2, 0, sizeof(sec2));
	n = blsSecretKeySerialize(buf, sizeof(buf), &sec1);
	CYBOZU_TEST_EQUAL(n, fpSize);
	ret = blsSecretKeyDeserialize(&sec2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSecretKeyIsEqual(&sec1, &sec2));

	blsPublicKey pub1, pub2;
	blsGetPublicKey(&pub1, &sec1);
	n = blsPublicKeySerialize(buf, sizeof(buf), &pub1);
	CYBOZU_TEST_EQUAL(n, fpSize * 2);
	ret = blsPublicKeyDeserialize(&pub2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsPublicKeyIsEqual(&pub1, &pub2));
	blsSignature sig1, sig2;
	blsSign(&sig1, &sec1, msg, msgSize);
	n = blsSignatureSerialize(buf, sizeof(buf), &sig1);
	CYBOZU_TEST_EQUAL(n, fpSize);
	ret = blsSignatureDeserialize(&sig2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSignatureIsEqual(&sig1, &sig2));
}

void blsOrderTest(const char *curveOrder, const char *fieldOrder)
{
	char buf[1024];
	size_t len;
	len = blsGetCurveOrder(buf, sizeof(buf));
	CYBOZU_TEST_ASSERT(len > 0);
	CYBOZU_TEST_EQUAL(buf, curveOrder);
	len = blsGetFieldOrder(buf, sizeof(buf));
	CYBOZU_TEST_ASSERT(len > 0);
	CYBOZU_TEST_EQUAL(buf, fieldOrder);
}

#if defined(CYBOZU_CPP_VERSION) && CYBOZU_CPP_VERSION >= CYBOZU_CPP_VERSION_CPP11
#include <thread>
#include <vector>
struct Thread {
	std::unique_ptr<std::thread> t;
	Thread() : t() {}
	~Thread()
	{
		if (t) {
			t->join();
		}
	}
	template<class F>
	void run(F func, int p1, int p2)
	{
		t.reset(new std::thread(func, p1, p2));
	}
};
CYBOZU_TEST_AUTO(multipleInit)
{
	const size_t n = 100;
	{
		std::vector<Thread> vt(n);
		for (size_t i = 0; i < n; i++) {
			vt[i].run(blsInit, mclBn_CurveFp254BNb, MCLBN_FP_UNIT_SIZE);
		}
	}
	CYBOZU_TEST_EQUAL(blsGetOpUnitSize(), 4u);
#if MCLBN_FP_UNIT_SIZE == 6
	{
		std::vector<Thread> vt(n);
		for (size_t i = 0; i < n; i++) {
			vt[i].run(blsInit, mclBn_CurveFp382_1, MCLBN_FP_UNIT_SIZE);
		}
	}
	CYBOZU_TEST_EQUAL(blsGetOpUnitSize(), 6u);
#endif
}
#endif

void blsSerializeTest()
{
	const size_t opUnitSize = mclBn_getOpUnitSize();
	printf("opUnitSize=%d\n", (int)opUnitSize);
	blsId id1, id2;
	blsSecretKey sec1, sec2;
	blsPublicKey pub1, pub2;
	blsSignature sig1, sig2;
	char buf[1024];
	size_t n;
	size_t expectSize;
	int ret;
	const char dummyChar = '1';

	// Id
	expectSize = opUnitSize * 8;
	blsIdSetInt(&id1, -1);
	n = blsIdSerialize(buf, sizeof(buf), &id1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	ret = blsIdDeserialize(&id2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsIdIsEqual(&id1, &id2));

	ret = blsIdDeserialize(&id2, buf, n - 1);
	CYBOZU_TEST_EQUAL(ret, 0);

	memset(&id2, 0, sizeof(id2));
	buf[n] = dummyChar;
	ret = blsIdDeserialize(&id2, buf, n + 1);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsIdIsEqual(&id1, &id2));

	n = blsIdSerialize(buf, expectSize, &id1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	// SecretKey
	expectSize = opUnitSize * 8;
	blsSecretKeySetDecStr(&sec1, "-1", 2);
	n = blsSecretKeySerialize(buf, sizeof(buf), &sec1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	ret = blsSecretKeyDeserialize(&sec2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSecretKeyIsEqual(&sec1, &sec2));

	ret = blsSecretKeyDeserialize(&sec2, buf, n - 1);
	CYBOZU_TEST_EQUAL(ret, 0);

	memset(&sec2, 0, sizeof(sec2));
	buf[n] = dummyChar;
	ret = blsSecretKeyDeserialize(&sec2, buf, n + 1);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSecretKeyIsEqual(&sec1, &sec2));

	n = blsSecretKeySerialize(buf, expectSize, &sec1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	// PublicKey
	expectSize = opUnitSize * 8 * 2;
	blsGetPublicKey(&pub1, &sec1);
	n = blsPublicKeySerialize(buf, sizeof(buf), &pub1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	ret = blsPublicKeyDeserialize(&pub2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsPublicKeyIsEqual(&pub1, &pub2));

	ret = blsPublicKeyDeserialize(&pub2, buf, n - 1);
	CYBOZU_TEST_EQUAL(ret, 0);

	memset(&pub2, 0, sizeof(pub2));
	buf[n] = dummyChar;
	ret = blsPublicKeyDeserialize(&pub2, buf, n + 1);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsPublicKeyIsEqual(&pub1, &pub2));

	n = blsPublicKeySerialize(buf, expectSize, &pub1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	// Signature
	expectSize = opUnitSize * 8;
	blsSign(&sig1, &sec1, "abc", 3);
	n = blsSignatureSerialize(buf, sizeof(buf), &sig1);
	CYBOZU_TEST_EQUAL(n, expectSize);

	ret = blsSignatureDeserialize(&sig2, buf, n);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSignatureIsEqual(&sig1, &sig2));

	ret = blsSignatureDeserialize(&sig2, buf, n - 1);
	CYBOZU_TEST_EQUAL(ret, 0);

	memset(&sig2, 0, sizeof(sig2));
	buf[n] = dummyChar;
	ret = blsSignatureDeserialize(&sig2, buf, n + 1);
	CYBOZU_TEST_EQUAL(ret, n);
	CYBOZU_TEST_ASSERT(blsSignatureIsEqual(&sig1, &sig2));

	n = blsSignatureSerialize(buf, expectSize, &sig1);
	CYBOZU_TEST_EQUAL(n, expectSize);
}

CYBOZU_TEST_AUTO(all)
{
	const int tbl[] = {
		mclBn_CurveFp254BNb,
#if MCLBN_FP_UNIT_SIZE == 6
		mclBn_CurveFp382_1,
		mclBn_CurveFp382_2
#endif
	};
	const char *curveOrderTbl[] = {
		"16798108731015832284940804142231733909759579603404752749028378864165570215949",
		"5540996953667913971058039301942914304734176495422447785042938606876043190415948413757785063597439175372845535461389",
		"5541245505022739011583672869577435255026888277144126952448297309161979278754528049907713682488818304329661351460877",
	};
	const char *fieldOrderTbl[] = {
		"16798108731015832284940804142231733909889187121439069848933715426072753864723",
		"5540996953667913971058039301942914304734176495422447785045292539108217242186829586959562222833658991069414454984723",
		"5541245505022739011583672869577435255026888277144126952450651294188487038640194767986566260919128250811286032482323",
	};
	for (size_t i = 0; i < sizeof(tbl) / sizeof(tbl[0]); i++) {
		printf("i=%d\n", (int)i);
		blsInit(tbl[i], MCLBN_FP_UNIT_SIZE);
		bls_use_stackTest();
		blsDataTest();
		blsOrderTest(curveOrderTbl[i], fieldOrderTbl[i]);
		blsSerializeTest();
	}
}
