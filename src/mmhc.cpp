#include <Rcpp.h>
#include <cstdlib>
#include <tr1/unordered_map>

using namespace std;
using namespace std::tr1;
using namespace Rcpp;

class MMHC {
	private:
		int vDim, hDim;
		double alpha, eta, score;
		List pc;
		IntegerMatrix A, graph;
		IntegerVector cardinality;
		NumericVector scores;
	public:
		MMHC(SEXP);
		~MMHC();

		SEXP GetPC() {return this->pc;}
		double GetScore() {return this->score;}
		SEXP GetGraph() {return this->graph;}

		template <typename T, int RTYPE> int colCardinality(const Vector<RTYPE>& x, unordered_map<T, int>& y);
		void Cardinality();
		int Hash(IntegerVector&, int, bool);
		double* OneD(double);
		double** TwoD(double, double);
		double*** ThreeD(double, double, double);
		double**** FourD(double, double, double, double);
		double***** FiveD(double, double, double, double, double);
		NumericVector Svalue(IntegerMatrix&, const IntegerVector&);
		IntegerMatrix partialMatrix(const IntegerVector&);
		IntegerVector CorrespondingCardinality(const IntegerVector&);
		IntegerVector SetCols(const IntegerVector&, int, int);
		void UpdateCPC(List&, double);
		bool IsIn(const IntegerVector&, double);
		void MaxMinHeuristic(const IntegerVector&, List&, IntegerVector&, int);
		void CompatibilityToR(IntegerVector&);
		List Forward(int);
		IntegerVector Backward(List&, int);

		unordered_map<int, int> UniqueMap(IntegerMatrix&);
		int getSingleN_ijk(IntegerVector&, int);
		int getVecN_ijk(IntegerVector&, IntegerVector&, int, int);
		int getMapN_ijk(IntegerVector&, IntegerMatrix&, unordered_map<int, int>, int, int);
		double ScoreNodeWithNoneParents(IntegerVector&, int);
		double ScoreNodeWithOneParent(IntegerVector&, IntegerVector&, int, int);
		double ScoreNodeWithMoreParents(IntegerVector&, IntegerMatrix&, int, int);
		NumericVector ReturnParents(int, IntegerMatrix&);
		void InitScore();
		void ScoreGraph(IntegerMatrix&, NumericVector&);
		void SettingEdges();
		void AddReverseDelete(IntegerMatrix&, NumericVector&);

		void mmpc();
		void mmhc();
};

MMHC::MMHC(SEXP x) {
	DataFrame B(x);
	this->vDim = B.nrows();
	this->hDim = B.length();
	this->A = IntegerMatrix(this->vDim, this->hDim);
	this->graph = IntegerMatrix(this->hDim, this->hDim);
	this->scores = NumericVector(this->hDim);
	IntegerVector tmp;
	for (int i = 0; i < this->hDim; i++) {
		tmp = B[i];
		this->A(_, i) = tmp;
	}
	this->alpha = 0.05;
	this->eta = 1.0;
	// this->eta = (double)(sum(this->cardinality))/(double)(this->cardinality.size());
	this->cardinality = IntegerVector(this->hDim, 0);
	Cardinality();
}

MMHC::~MMHC() {}

template <typename T, int RTYPE> int MMHC::colCardinality(const Vector<RTYPE>& x, unordered_map<T, int>& y) {
	int m = x.size();
	y.clear();
	for (int i = 0; i < m; i++)
		y[x[i]] = 1;

	return y.size();
}

void MMHC::Cardinality() {
	IntegerVector x;
	unordered_map<int, int> y;
	for (int i = 0; i < this->hDim; i++) {
		x = this->A(_, i);
		this->cardinality[i] = colCardinality<int, INTSXP>(x, y);
	}
}

int MMHC::Hash(IntegerVector& array, int i, bool skip) {
	int out = 0;
	for (i; i < array.size(); i++) {
		if (skip && i == 1)
			continue;
		out += 10*out + array[i];
	}

	return out;
}

double *MMHC::OneD(double x) {
	double *matrix = (double*)calloc(x, sizeof(double));
	for (int i = 0; i < x; i++)
		matrix[i] = 0;
	return matrix;
}

double **MMHC::TwoD(double x, double y) {
	double **matrix = (double**)calloc(x, sizeof(double*));
	for (int i = 0; i < x; i++)	{
		matrix[i] = (double*)calloc(y, sizeof(double));
		for (int j = 0; j < y; j++)
			matrix[i][j] = 0;
	}
	return matrix;
}

double ***MMHC::ThreeD(double x, double y, double z) {
	double ***matrix = (double***)calloc(x, sizeof(double*));
	for (int i = 0; i < x; i++)	{
		matrix[i] = (double**)calloc(y, sizeof(double*));
		for (int j = 0; j < y; j++)	{
			matrix[i][j] = (double*)calloc(z, sizeof(double));
			for (int k = 0; k < z; k++)
				matrix[i][j][k] = 0;
		}
	}
	return matrix;
}

double ****MMHC::FourD(double x, double y, double z, double a) {
	double ****matrix = (double****)calloc(x, sizeof(double*));
	for (int i = 0; i < x; i++)	{
		matrix[i] = (double***)calloc(y, sizeof(double*));
		for (int j = 0; j < y; j++)	{
			matrix[i][j] = (double**)calloc(z, sizeof(double*));
			for (int k = 0; k < z; k++)	{
				matrix[i][j][k] = (double*)calloc(a, sizeof(double));
				for (int l = 0; l < a; l++)
					matrix[i][j][k][l] = 0;
			}
		}
	}
	return matrix;
}

double *****MMHC::FiveD(double x, double y, double z, double a, double b) {
	double *****matrix = (double*****)calloc(x, sizeof(double*));
	for (int i = 0; i < x; i++)	{
		matrix[i] = (double****)calloc(y, sizeof(double*));
		for (int j = 0; j < y; j++)	{
			matrix[i][j] = (double***)calloc(z, sizeof(double*));
			for (int k = 0; k < z; k++)	{
				matrix[i][j][k] = (double**)calloc(a, sizeof(double*));
				for (int l = 0; l < a; l++)	{
					matrix[i][j][k][l] = (double*)calloc(b, sizeof(double));
					for (int m = 0; m < b; m++)
						matrix[i][j][k][l][m] = 0;
				}
			}
		}
	}
	return matrix;
}

NumericVector MMHC::Svalue(IntegerMatrix& A, const IntegerVector& cardinality) {
	int hDim = A.ncol(), vDim = A.nrow();
	NumericVector sum(1, 0.0), pvalue(1, 0.0), out(2, 0.0);

	if (hDim == 2) {
		int l = cardinality[1], m = cardinality[0];
		double *x = OneD(m), *y = OneD(l), **z = TwoD(m, l);

		for (int i = 0; i < vDim; i++) {
			x[(int)A(i, 0) - 1]++;
			y[(int)A(i, 1) - 1]++;
			z[(int)A(i, 0) - 1][(int)A(i, 1) - 1]++;
		}

		for (int i = 0; i < m; i++) {
			for (int j = 0; j < l; j++) {
				if (x[i] < 1 || y[j] < 1 || z[i][j] < 1)
					continue;
				sum[0] += 2.0 * z[i][j] * log( (z[i][j] * vDim) / (x[i] * y[j]) );
			}
		}

		for (int i = 0; i < m; i++)
			free(z[i]);

		free(z);
		free(x);
		free(y);

		int DF = cardinality[0] * cardinality[1];
		for (int i = 2; i < cardinality.size(); i++)
			DF *= (cardinality[i] + 1);

		pvalue = pchisq(sum, DF, FALSE);
		out[0] = pvalue[0];
		out[1] = sum[0];

		return out;

	}  else if (hDim == 3) {
		int k = cardinality[2], l = cardinality[1], m = cardinality[0];
		double *v = OneD(k), **x = TwoD(m, k), **y = TwoD(l, k), ***z = ThreeD(m, l, k);

		for (int i = 0; i < vDim; i++) {
			v[(int)A(i, 2) - 1]++;
			x[(int)A(i, 0) - 1][(int)A(i, 2) - 1]++;
			y[(int)A(i, 1) - 1][(int)A(i, 2) - 1]++;
			z[(int)A(i, 0) - 1][(int)A(i, 1) - 1][(int)A(i, 2) - 1]++;
		}

		for (int i = 0; i < m; i++) {
			for (int j = 0; j < l; j++) {
				for (int h = 0; h < k; h++) {
					if (x[i][h] < 1 || y[j][h] < 1 || z[i][j][h] < 1 || v[h] < 1)
						continue;
					sum[0] += 2.0 * z[i][j][h] * log( (z[i][j][h] * v[h]) / (x[i][h] * y[j][h]) );
				}
			}
		}

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < l; j++)
			{
				free(z[i][j]);
			}
			free(z[i]);
		}
		free(z);

		for (int i = 0; i < l; i++)
			free(y[i]);
		free(y);

		for (int i = 0; i < m; i++)
			free(x[i]);
		free(x);

		free(v);

		int DF = cardinality[0] * cardinality[1];
		for (int i = 2; i < cardinality.size(); i++)
			DF *= (cardinality[i] + 1);

		pvalue = pchisq(sum, DF, FALSE);
		out[0] = pvalue[0];
		out[1] = sum[0];

		return out;

	} else if (hDim == 4) {
		int n = cardinality[3], k = cardinality[2], l = cardinality[1], m = cardinality[0];
		double **v = TwoD(k, n), ***x = ThreeD(m, k, n), ***y = ThreeD(l, k, n), ****z = FourD(m, l, k, n);

		for (int i = 0; i < vDim; i++) {
			v[(int)A(i, 2) - 1][(int)A(i, 3) - 1]++;
			x[(int)A(i, 0) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1]++;
			y[(int)A(i, 1) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1]++;
			z[(int)A(i, 0) - 1][(int)A(i, 1) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1]++;
		}

		for (int i = 0; i < m; i++) {
			for (int j = 0; j < l; j++) {
				for (int h = 0; h < k; h++) {
					for (int f = 0; f < n; f++) {
						if (x[i][h][f] < 1 || y[j][h][f] < 1 || z[i][j][h][f] < 1 || v[h][f] < 1)
							continue;
						sum[0] += 2.0 * z[i][j][h][f] * log( (z[i][j][h][f] * v[h][f]) / (x[i][h][f] * y[j][h][f]) );
					}
				}
			}
		}

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < l; j++)
			{
				for (int a = 0; a < k; a++)
				{
					free(z[i][j][a]);
				}
				free(z[i][j]);
			}
			free(z[i]);
		}
		free(z);

		for (int i = 0; i < l; i++)
		{
			for (int j = 0; j < k; j++)
			{
				free(y[i][j]);
			}
			free(y[i]);
		}
		free(y);

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < k; j++)
			{
				free(x[i][j]);
			}
			free(x[i]);
		}
		free(x);

		for (int i = 0; i < k; i++)
			free(v[i]);
		free(v);


		int DF = cardinality[0] * cardinality[1];
		for (int i = 2; i < cardinality.size(); i++)
			DF *= (cardinality[i] + 1);

		pvalue = pchisq(sum, DF, FALSE);
		out[0] = pvalue[0];
		out[1] = sum[0];

		return out;

	} else if (hDim == 5) {
		int o = cardinality[4], n = cardinality[3], k = cardinality[2], l = cardinality[1], m = cardinality[0];
		double ***v = ThreeD(k, n, o), ****x = FourD(m, k, n, o), ****y = FourD(l, k, n, o), *****z = FiveD(m, l, k, n, o);

		for (int i = 0; i < vDim; i++) {
			v[(int)A(i, 2) - 1][(int)A(i, 3) - 1][(int)A(i, 4) - 1]++;
			x[(int)A(i, 0) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1][(int)A(i, 4) - 1]++;
			y[(int)A(i, 1) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1][(int)A(i, 4) - 1]++;
			z[(int)A(i, 0) - 1][(int)A(i, 1) - 1][(int)A(i, 2) - 1][(int)A(i, 3) - 1][(int)A(i, 4) - 1]++;
		}

		for (int i = 0; i < m; i++) {
			for (int j = 0; j < l; j++) {
				for (int h = 0; h < k; h++) {
					for (int f = 0; f < n; f++) {
						for (int e = 0; e < o; e++) {
							if (x[i][h][f][e] < 1 || y[j][h][f][e] < 1 || z[i][j][h][f][e] < 1 || v[h][f][e] < 1)
								continue;
							sum[0] += 2.0 * z[i][j][h][f][e] * log( (z[i][j][h][f][e] * v[h][f][e]) / (x[i][h][f][e] * y[j][h][f][e]) );
						}
					}
				}
			}
		}

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < l; j++)
			{
				for (int a = 0; a < k; a++)
				{
					for (int b = 0; b < n; b++)
					{
						free(z[i][j][a][b]);
					}
					free(z[i][j][a]);
				}
				free(z[i][j]);
			}
			free(z[i]);
		}
		free(z);

		for (int i = 0; i < l; i++)
		{
			for (int j = 0; j < k; j++)
			{
				for (int a = 0; a < n; a++)
				{
					free(y[i][j][a]);
				}
				free(y[i][j]);
			}
			free(y[i]);
		}
		free(y);

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < k; j++)
			{
				for (int a = 0; a < n; a++)
				{
					free(x[i][j][a]);
				}
				free(x[i][j]);
			}
			free(x[i]);
		}
		free(x);

		for (int i = 0; i < k; i++)
		{
			for (int j = 0; j < n; j++)
			{
				free(v[i][j]);
			}
			free(v[i]);
		}
		free(v);

		int DF = cardinality[0] * cardinality[1];
		for (int i = 2; i < cardinality.size(); i++)
			DF *= (cardinality[i] + 1);

		pvalue = pchisq(sum, DF, FALSE);
		out[0] = pvalue[0];
		out[1] = sum[0];

		return out;
		
	} else {
		// int acKey, bcKey, abcKey, cKey;
		// unordered_map<int, int> Count;
		// unordered_map<int, int> ReMap;
		// int teta = 0;
		// IntegerVector tmp(hDim);

		// for (int i = 0; i < hDim; i++) {
		// 	tmp = A(i, _);
		// 	abcKey = Hash(tmp, 0, FALSE);
		// 	bcKey = Hash(tmp, 1, FALSE);
		// 	cKey = Hash(tmp, 2, FALSE);
		// 	acKey = Hash(tmp, 0, TRUE);
		// 	if (Count[abcKey] == 0) {
		// 		ReMap[teta] = abcKey;
		// 		ReMap[teta+1] = acKey;
		// 		ReMap[teta+2] = bcKey;
		// 		ReMap[teta+3] = cKey;
		// 		teta = teta + 4;
		// 		Count[abcKey]++;
		// 		Count[acKey]++;
		// 		Count[bcKey]++;
		// 		Count[cKey]++;
		// 	}
		// }

		// for (int i = 0; i < ReMap.size(); i = i + 4)
		// 	sum[0] += 2.0 * Count[ReMap[i]] == 0 * log( (Count[ReMap[i]] == 0 * Count[ReMap[i+3]] == 0) / (Count[ReMap[i+1]] == 0 * Count[ReMap[i+2]] == 0) );

		// int DF = cardinality[0] * cardinality[1];
		// for (int i = 2; i < cardinality.size(); i++)
		// 	DF *= (cardinality[i] + 1);

		// pvalue = pchisq(sum, DF, FALSE);
		// out[0] = pvalue[0];
		// out[1] = sum[0];

		return NumericVector::create(1.0, 1.0);

	}
}

IntegerMatrix MMHC::partialMatrix(const IntegerVector& pa) {
	IntegerMatrix partMat(this->vDim, pa.size());
	for (int i = 0; i < pa.size(); i++)
		partMat(_, i) = this->A(_, pa(i));
	return partMat;
}

IntegerVector MMHC::CorrespondingCardinality(const IntegerVector& pa) {
	IntegerVector tmpCardinality;
	for (int i = 0; i < pa.size(); i++)
		tmpCardinality.push_back(this->cardinality[pa[i]]);
	return tmpCardinality;
}

IntegerVector MMHC::SetCols(const IntegerVector& cpc, int T, int X) {
	IntegerVector pa;
	pa.push_back(X);
	pa.push_back(T);
	for (int i = 0; i < cpc.size(); i++)
		pa.push_back(cpc[i]);
	return pa;
}

void MMHC::UpdateCPC(List& CPC, double selected) {
	IntegerVector tmp;
	if (CPC.size() == 0) {
		CPC.push_back(0);
		CPC.push_back(R_NilValue);
	} else if (CPC.size() == 2) {
		CPC[0] = 2;
		CPC.push_back(selected);
	} else {
		int CPCLength = CPC.size();
		CPC[0] = CPCLength;
		CPC.push_back(selected);
		for (int i = 2; i < CPCLength; i++) {
			tmp = as<IntegerVector>(CPC[i]);
			tmp.push_back(selected);
			CPC.push_back(tmp);
		}
	}
}

bool MMHC::IsIn(const IntegerVector& x, double y) {
	bool out = FALSE;
	for (int i = 0; i < x.size(); i++) {
		if (x[i] == y) {
			out = TRUE;
			break;
		}
	}
	return out;
}

void MMHC::MaxMinHeuristic(const IntegerVector& cpc, List& CPCprops, IntegerVector& variablesToTest, int T) {
	IntegerMatrix statisticMatrix;
	IntegerVector pa, tmpCardinality, rejectedInLastStep = as<IntegerVector>(CPCprops[1]), temporaryMinimum = rejectedInLastStep, accepted;
	NumericVector pvalue;

	for (int i = 0; i < variablesToTest.size(); i++) {
		pa = SetCols(cpc, T, variablesToTest[i]);
		tmpCardinality = CorrespondingCardinality(pa);
		statisticMatrix = partialMatrix(pa);
		pvalue = Svalue(statisticMatrix, tmpCardinality);

		if (pvalue[0] < this->alpha) {
			if (pvalue[0] < rejectedInLastStep[1]) {
				temporaryMinimum = rejectedInLastStep;
				rejectedInLastStep = IntegerVector::create(variablesToTest[i], pvalue[0], pvalue[1]);
			} else if (pvalue[0] == rejectedInLastStep[1] && pvalue[1] > rejectedInLastStep[2]) {
				rejectedInLastStep = IntegerVector::create(variablesToTest[i], rejectedInLastStep[1], pvalue[1]);
			}
		} else {
			accepted.push_back(variablesToTest[i]);
		}
	}

	if (accepted.size() > 0)
		CPCprops[0] = accepted;
	if (rejectedInLastStep[0] != -1)
		CPCprops[1] = rejectedInLastStep;
	if (temporaryMinimum[0] != -1 && temporaryMinimum[0] != rejectedInLastStep[0])
		CPCprops[2] = temporaryMinimum;
}

void MMHC::CompatibilityToR(IntegerVector& cpc) {
	for (int i = 0; i < cpc.size(); i++)
		cpc[i]++;
}

List MMHC::Forward(int T) {
	IntegerVector cpc, variablesToTest;
	List CPC, CPCprops;
	
	IntegerVector reject = IntegerVector::create(-1.0, 1.0, 1.0), accepted = IntegerVector::create(T), tmpAccepted;
	CPCprops.push_back(T);
	CPCprops.push_back(reject);
	CPCprops.push_back(reject);

	for (int i = 0; i < this->hDim; i++)
		if (i != T)
			variablesToTest.push_back(i);

	UpdateCPC(CPC, 0);
	MaxMinHeuristic(cpc, CPCprops, variablesToTest, T);
	
	reject = as<IntegerVector>(CPCprops[1]);
	tmpAccepted = as<IntegerVector>(CPCprops[0]);

	if (reject[0] != -1.0) {
		UpdateCPC(CPC, reject[0]);
		cpc.push_back(reject[0]);
		accepted.push_back(reject[0]);
	}

	for (int i = 0; i < tmpAccepted.size(); i++)
		accepted.push_back(tmpAccepted[i]);

	variablesToTest = IntegerVector::create();
	for (int i = 0; i < this->hDim; i++)
	{
		if (!IsIn(accepted, i)) {
			variablesToTest.push_back(i);
		}
	}

	while (variablesToTest.size() > 0) {
		CPCprops[1] = IntegerVector::create(-1.0, 1.0, 1.0);
		MaxMinHeuristic(cpc, CPCprops, variablesToTest, T);

		reject = as<IntegerVector>(CPCprops[1]);
		tmpAccepted = as<IntegerVector>(CPCprops[0]);

		if (reject[0] != -1.0) {
			UpdateCPC(CPC, reject[0]);
			cpc.push_back(reject[0]);
			accepted.push_back(reject[0]);

			for (int i = 0; i < tmpAccepted.size(); i++)
				accepted.push_back(tmpAccepted[i]);

			variablesToTest = IntegerVector::create();
			for (int i = 0; i < this->hDim; i++)
			{
				if (!IsIn(accepted, i)) {
					variablesToTest.push_back(i);
				}
			}
		} else {
			variablesToTest = IntegerVector::create();
		}
	}

	return CPC;
}

IntegerVector MMHC::Backward(List& CPC, int T) {
	IntegerVector cpc = as<IntegerVector>(CPC[CPC.size()-1]), rm, pa, tmpCardinality, fromCPC;
	NumericVector pvalue;
	IntegerMatrix statisticMatrix;
	int k = -1;

	if (cpc.size() == 1) {
		return cpc;
	} else {
		for (int i = 0; i < cpc.size(); i++) {
			for (int j = 1; j < CPC.size(); j++) {
				if (j == 1) {
					fromCPC = IntegerVector::create();
				} else {
					fromCPC = as<IntegerVector>(CPC[j]);
				}

				pa = SetCols(fromCPC, T, cpc[i]);
				tmpCardinality = CorrespondingCardinality(pa);
				statisticMatrix = partialMatrix(pa);
				pvalue = Svalue(statisticMatrix, tmpCardinality);

				if (pvalue[0] > this->alpha && pvalue[0] != 1.0) {
					k = i;
					break;
				}
			}
		}

		if (k != -1)
			cpc.erase(k);

		return cpc;
	}
}

unordered_map<int, int> MMHC::UniqueMap(IntegerMatrix& A) {
	IntegerVector u;
	int key;
	unordered_map<int, int> Map;
	unordered_map<int, int> Uni;
	int n = Map.size(), k = 0;

	for (int i = 0; i < this->vDim; i++)
	{
		u = A(i, _);
		key = Hash(u, 0, FALSE);
		Map[key] = 1;
		if (Map.size() != n) {
			Uni[k] = key;
			n++;
			k++;
		}
	}
	return Uni;
}

int MMHC::getSingleN_ijk(IntegerVector& vec, int k) {
	int count = 0;
	for (int i = 0; i < vec.size(); i++)
		if (vec[i] == k)
			count++;
	return count;
}

int MMHC::getVecN_ijk(IntegerVector& vec, IntegerVector& parentVec, int j, int k) {
	int count = 0;
	for (int i = 0; i < vec.size(); i++)
		if (vec[i] == k && parentVec[i] == j)
			count++;
	return count;
}

int MMHC::getMapN_ijk(IntegerVector& vec, IntegerMatrix& parentMatrix, unordered_map<int, int> parentMap, int j, int k) {
	int count = 0;
	IntegerVector u;
	for (int i = 0; i < vec.size(); i++) {
		u = parentMatrix(i, _);
		if (vec[i] == k && parentMap[j-1] == Hash(u, 0, FALSE))
			count++;
	}
	return count;
}

double MMHC::ScoreNodeWithNoneParents(IntegerVector& vec, int r) {
	double gammaJ = this->eta / 1.0, gammaK = this->eta / (1.0 * r);
	double rScore = 0.0, qScore = 0.0;
	int n_ijk = 0, n_ij = 0;

	for (int k = 1; k <= r; k++) {
		n_ijk = getSingleN_ijk(vec, k);
		n_ij += n_ijk;
		if (n_ijk != 0)
			rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
	}
	qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;
	return qScore;
}

double MMHC::ScoreNodeWithOneParent(IntegerVector& vec, IntegerVector& parentVec, int r, int q) {
	double gammaJ = this->eta / q, gammaK = this->eta / (q * r);
	double rScore = 0.0, qScore = 0.0;
	int n_ij, n_ijk = 0;

	for (int j = 1; j <= q; j++) {
		n_ij = 0;
		rScore = 0.0;
		for (int k = 1; k <= r; k++) {
			n_ijk = getVecN_ijk(vec, parentVec, j, k);
			n_ij += n_ijk;
			if (n_ijk != 0)
				rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
		}
		qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;
	}
	return qScore;
}

double MMHC::ScoreNodeWithMoreParents(IntegerVector& vec, IntegerMatrix& Parent, int r, int q) {
	unordered_map<int, int> parentMap = UniqueMap(Parent);
	double gammaJ = this->eta / q, gammaK = this->eta / ( q * r);
	double rScore = 0.0, qScore = 0.0;
	int n_ij, n_ijk;

	for (int j = 1; j <= q; j++) {
		n_ij = 0;
		rScore = 0.0;
		for (int k = 1; k <= r; k++) {
			n_ijk = getMapN_ijk(vec, Parent, parentMap, j, k);
			n_ij += n_ijk;
			if (n_ijk != 0)
				rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
		}
		qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;
	}
	return qScore;
}

NumericVector MMHC::ReturnParents(int i, IntegerMatrix& AdjMat) {
	NumericVector parents;
	for (int j = 0; j < this->hDim; j++)
		if (AdjMat(j, i) == 1)
			parents.push_back(j);
	return parents;
}

void MMHC::InitScore() {
	IntegerVector g;
	for (int i = 0; i < this->hDim; i++) {
		g = this->A(_, i);
		this->scores[i] = ScoreNodeWithNoneParents(g, this->cardinality[i]);
	}
}

void MMHC::ScoreGraph(IntegerMatrix& AdjMat, NumericVector& tmp) {
	IntegerMatrix parentMatrix;
	IntegerVector childVector, parentVector, allParents;
	int q;

	for (int i = 0; i < this->hDim; i++) {
		q = 1;
		allParents = ReturnParents(i, AdjMat);
		childVector = this->A(_, i);

		for (int k = 0; k < allParents.size(); k++)
			q *= this->cardinality[allParents[k]];

		if (allParents.size() == 0) {
			tmp[i] = ScoreNodeWithNoneParents(childVector, this->cardinality[i]);
		} else if (allParents.size() == 1) {
			parentVector = this->A(_, allParents[0]);
			tmp[i] = ScoreNodeWithOneParent(childVector, parentVector, this->cardinality[i], q);
		} else {
			parentMatrix = partialMatrix(allParents);
			tmp[i] = ScoreNodeWithMoreParents(childVector, parentMatrix, this->cardinality[i], q);
		}
	}
}

void MMHC::SettingEdges() {
	NumericVector tmp(this->hDim);
	double before, after;

	for (int i = 0; i < this->pc.size(); i++) {
		IntegerVector subPC = as<IntegerVector>(this->pc[i]);
		for (int j = 0; j < subPC.size(); j++) {
			if (this->graph(i, subPC[j] - 1) == 0 && this->graph(subPC[j] - 1, i) == 0) {
				this->graph(i, subPC[j] - 1) = 1;
				before = sum(this->scores);
				ScoreGraph(this->graph, tmp);
				after = sum(tmp);

				if (after > before) {
					this->scores = tmp;
				} else {
					this->graph(i, subPC[j] - 1) = 0;
				}
			}
		}
	}
}

void MMHC::AddReverseDelete(IntegerMatrix& AdjMat, NumericVector& scores) {
	srand(time(NULL));
	NumericVector tmp(this->hDim);
	double before, after;
	int rnd;

	for (int i = 0; i < this->pc.size(); i++) {
		IntegerVector subPC = as<IntegerVector>(this->pc[i]);
		for (int j = 0; j < subPC.size(); j++) {
			rnd = (rand() % 10) + 1;
			if (AdjMat(i, subPC[j] - 1) == 0 && AdjMat(subPC[j] - 1, i) == 0) {
				AdjMat(subPC[j] - 1, i) = 1;
				before = sum(scores);
				ScoreGraph(AdjMat, tmp);
				after = sum(tmp);

				if (after > before) {
					scores = tmp;
				} else {
					AdjMat(subPC[j] - 1, i) = 0;
				}
			} else if (AdjMat(i, subPC[j] - 1) == 1 && rnd > 5) {
				AdjMat(i, subPC[j] - 1) = 0;
				before = sum(scores);
				ScoreGraph(AdjMat, tmp);
				after = sum(tmp);

				if (after > before) {
					scores = tmp;
				} else {
					AdjMat(i, subPC[j] - 1) = 1;
				}
			}  else if (AdjMat(subPC[j] - 1, i) == 1 && rnd <= 5) {
				AdjMat(subPC[j] - 1, i) = 0;
				AdjMat(i, subPC[j] - 1) = 1;
				before = sum(scores);
				ScoreGraph(AdjMat, tmp);
				after = sum(tmp);

				if (after > before) {
					scores = tmp;
				} else {
					AdjMat(subPC[j] - 1, i) = 1;
					AdjMat(i, subPC[j] - 1) = 0;
				}
			}
		}
	}
}

void MMHC::mmpc() {
	IntegerVector cpc, pcVec, tmppc;
	List CPC, tmpPC;
	
	for (int T = 0; T < 5; T++) {
		CPC = Forward(T);
		cpc = as<IntegerVector>(CPC[CPC.size()-1]);
		if (cpc.size() == 0) {
			this->pc.push_back(R_NilValue);
		} else {
			pcVec = Backward(CPC, T);
			for (int j = 0; j < pcVec.size(); j++) {
				tmpPC = Forward((int)pcVec[j]);
				tmppc = Backward(tmpPC, (int)pcVec[j]);
				if (!(IsIn(tmppc, T))) {
					for (int l = 0; l < pcVec.size(); l++) {
						if (pcVec[l] == pcVec[j]) {
							pcVec.erase(l);
						}
					}
				}
			}
			CompatibilityToR(pcVec);
			this->pc.push_back(pcVec);
		}
	}
}

void MMHC::mmhc() {
	NumericVector tmpScores;
	IntegerMatrix tmpAdjMat(this->hDim, this->hDim);

	InitScore();
	SettingEdges();
	AddReverseDelete(tmpAdjMat, tmpScores);

	int count = 1;
	for (int i = 0; i < 100; i++) {
		if (count == 5)
			break;

		tmpScores = this->scores;
		tmpAdjMat = this->graph;
		AddReverseDelete(tmpAdjMat, tmpScores);
		
		if (sum(this->scores) > sum(tmpScores)) {
			count++;
		} else {
			this->graph = tmpAdjMat;
			this->scores = tmpScores;
			count = 1;
		}
	}
	this->score = sum(this->scores);
}

RCPP_MODULE(mmhc) {
	class_<MMHC>("MMHC")
		.constructor<SEXP>()
		.method("pc", &MMHC::GetPC, "returns the PC set from MMPC()")
		.method("adjMat", &MMHC::GetGraph, "returns the adjacency from MMHC()")
		.method("score", &MMHC::GetScore, "return the score of the final graph")
		.method("mmpc", &MMHC::mmpc, "executes the MMPC() function")
		.method("mmhc", &MMHC::mmhc, "executes the MMHC() function")
	;
}