#ifndef ACCEPTANCE_H
#define ACCEPTANCE_H

#include <vector>
#include <string>

#include "HRGBase/BilinearSplineFunction.h"


namespace Acceptance {

	/**
	*	Structure which contains the binomial probabilties
	*	for particle with given y and pt to be accepted.
	*/
	struct AcceptanceFunction {
	  bool init;
	  double dy, dpt;
	  std::vector<double> ys, pts, probs;
	  BilinearSplineFunction sfunc;

		AcceptanceFunction(): dy(), dpt(), ys(), pts(), probs(), sfunc() { init = false; }

	  void setSpline() { sfunc.setData(ys, pts, probs); init = true; }
	  double getAcceptance(const double & y, const double & pt) const;
	};



	/**
	*	Read the acceptance function from file.
	*/
	int ReadAcceptanceFunction(AcceptanceFunction & func, std::string filename);
}

#endif
