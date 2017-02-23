#include <complex>
#include <cstring>
#include <cmath>
#include <omp.h>

typedef std::complex<double> complexd;
typedef unsigned long long ulong;
const MAX_BITS = sizeof(ulong) * 8;

using namespace std;

#include "assert.h"
#include "tools.h"

class BinaryIndex
{
	char *index;
	size_t n;
public:
	BinaryIndex(const size_t _n): n(_n)
	{
		assert(n != 0, "Empty bitset!");
		assert(n <= MAX_BITS, "Number of bits is too big", {{"Number of bits", n}, {"Max", MAX_BITS}});
		try
		{
			index = new char[n];
		}
		catch (bad_alloc& ba)
		{
			cerr << "New BinaryIndex could not be created: bad_alloc caught: " << ba.what() << '\n';
			exit(10);
		}
		memset(index, 0, n*sizeof(char));	// set the index initially to zero
	}
	~BinaryIndex()
	{
		assert(index != nullptr, "Already destructed!");
		delete index [];
	}
	void to_index(const ulong _k)
	{
		ulong k = _k;
		const int begin = (int)(n-1);
		for(int i = begin; i > -1; i--)
		{
			index[i] = k % 2;
			k /= 2;
		}
		assert(k == 0, "The value is too big for this index!", {{"Value", k}, {"Index size(bits)", n}});
	}
	ulong to_ulong_with_insert(const size_t bit_pos, const char bit_value) const
	{
		assert(bit_pos <= n && bit_pos >= 1, "Bit position is outside of possible range!", {{"Bit number", bit_pos}, {"Max bits", n}});
		size_t k = bit_pos - 1;
		ulong sum = 0;
		for(size_t i = 0; i < k; i++)
		{
			sum *= 2;
			sum += index[i];
		}
		sum *= 2;
		sum += bit_value;
		for(size_t i = k+1; i < n; i++)
		{
			sum *= 2;
			sum += index[i];
		}
		return sum;
	}
	ulong to_ulong() const
	{
		ulong sum = 0;
		for(size_t i = 0; i < n; i++)
		{
			sum *= 2;
			sum += index[i];
		}
		return sum;
	}
	BinaryIndex &flip(const size_t k)
	{
		assert(k >= 1 && k <= n, "Trying to flip a bit outside of possible range!", {{"Bit number", k}, {"Max bits", n}});
		size_t ind = k-1;
		if(index[ind] == 0)
			index[ind] = 1;
		else
			index[ind] = 0;
		return (*this);
	}
	BinaryIndex &add (const char c)
	{
		note(c != 0 && c != 1, "Adding more than one to index", {{"Value of c", c}});
		to_index(to_ulong() + c);
		return (*this);
	}
	char test(const size_t k) const
	{
		assert(k >= 1 && k <= n, "Trying to access a bit outside of possible range!", {{"Bit number", k}, {"Max bits", n}});
		return index[k];
	}
};

class QuantumState
{
	size_t qubits_n;
	vector<complexd> state;
	ulong size;
public:
	QuantumState(const size_t _qubits_n): qubits_n(_qubits_n)
	{
		assert(qubits_n >= 1 && qubits_n <= MAX_BITS, "Number of qubits is too big", {{"Number of qubits", qubits_n}, {"Max", sizeof(ulong)}});
		ulong maxsize = 0;
		{
			BinaryIndex ind(qubits_n);
			ulong size = ind.flip(1).to_ulong();
			maxsize = vector::max_size;
		}
		assert(size <= maxsize, "Vector is too long", {{"Vector size", size}, {"Max size", maxsize}});
		try
		{
			state = vector<complexd> (size);
		}
		catch (bad_alloc& ba)
		{
			cerr << "New QuantumState could not be created: bad_alloc caught: " << ba.what() << '\n';
			exit(20);
		}
		// OMP generate values
		#pragma omp parallel
		{
			Tools::srand(omp_get_thread_num());
			#pragma omp for
			for(ulong i = 0; i < size; i++)
			{
				double real, imag;
				real = Tools::rand();
				imag = Tools::rand();
				state[i] = complexd(real,imag);
			}
		}
	}
	void transform(const size_t k)
	{
		#pragma omp parallel
		{
			size_t rank = omp_get_thread_num();
			size_t thread_num = omp_get_num_threads();
			ulong multiplier = size/(2*thread_num);	// multiplier for binary index start position
			ulong sections_per_thread = multiplier;	// how many pairs each thread is processing
			BinaryIndex index(qubits_n-1);			// position in vector
			index.to_index(multiplier * rank);		// position for each thread to start
			for(ulong i = 0; i < sections_per_thread; i++)
			{
				ulong index1 = index.to_ulong_with_insert(k, 0);
				ulong index2 = index.to_ulong_with_insert(k, 1);
				complexd value1 = state[index1];
				complexd value2 = state[index2];
				complexd sum = (value1 + value2)/sqrt(2);
				complexd diff = (value1 - value2)/sqrt(2);
				state[index1] = sum;
				state[index2] = diff;
				
				index.add(1);
			}
		}
	}
	void print() const
	{
		cout << "Vector of size "<<size << endl;
		cout << "----------------" << endl;
		for(ulong i = 0; i < size; i++)
			cout << "v["<<i<<"]:\t" << state[i] << endl;
	}
};

int main(int argc, char **argv)
{
	// Tools::cls();
	// complexd water(2.3,3.2);
	// Printer::assert(water.real() < 2, "Wrong");
	// Printer::debug(true, "What's the value of water?", { {"Water value", water} });
	// Printer::note(water.imag() > 3, "Show this message in yellow");
	// Printer::refute(water, "True");

}