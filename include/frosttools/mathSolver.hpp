#pragma once

namespace frosttools
{
/*
A * X = B
*/
/// Result for symbolics solving
enum solverResult
{
	resultOk,
	resultNoSolution
};

/// Solves system of linear equations
template<typename Real>
int solveLinearSystem(int equationNumber,int variableCount,Real *a,Real *b,Real *x)
{
	MatrixNM<Real> matrix(variableCount,equationNumber,a);
	//matrix.writeCol(variableCount,x);
	//matrix.writeCol(variableCount+1,x);
	matrix.reduceToTriangle();
	return 0;
}

}
