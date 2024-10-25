import numpy as np

dim = 2
nperm = 1000
seeddata = 1991
seedquery = 1996

for a in [1,5,50]:
	np.random.seed(seeddata)
	data = np.random.power(a,nperm*dim).reshape((nperm,dim))
	f = open('data_Rand1k_pow'+str(a)+'_len'+str(dim)+'_org.bin', mode = 'wb')
	np.asarray(data, dtype = np.float32).tofile(f)
	f.close

	np.random.seed(seedquery)
	query = np.random.power(a,100*dim).reshape((100,dim))
	f = open('query_Rand100_pow'+str(a)+'_len'+str(dim)+'_org.bin', mode = 'wb')
	np.asarray(query, dtype = np.float32).tofile(f)
	f.close

