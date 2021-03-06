#include "HRGFit/ThermalModelFit.h"

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef USE_MINUIT
#include "Minuit2/FCNBase.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnHesse.h"
#include "Minuit2/MnUserParameterState.h"
//#include "Minuit2/MnPrint.h"
//#include "Minuit2/SimplexMinimizer.h"
#endif

#include "HRGBase/ThermalModelBase.h"


#ifdef USE_MINUIT

using namespace ROOT::Minuit2;

int migraditer = 0;

namespace ThermalModelFitNamespace {

	class FitFCN : public FCNBase {

	public:

		FitFCN(ThermalModelFit *thmfit_, bool verbose_ = true) : m_THMFit(thmfit_), m_verbose(verbose_) { 
			migraditer = 0;
		}

		~FitFCN() {}

		double operator()(const std::vector<double>& par) const {
			++migraditer;
			m_THMFit->Increment();
			double chi2 = 0.;
			if (par[2]<0.) return 1e12;
			if (par[3]<0.) return 1e12;

			m_THMFit->model()->SetTemperature(par[0]);

			m_THMFit->model()->SetBaryonChemicalPotential(par[1]);

			m_THMFit->model()->SetGammaS(par[2]);

			m_THMFit->model()->SetVolumeRadius(par[3]);

			if (m_THMFit->Parameters().Rc.toFit) m_THMFit->model()->SetStrangenessCanonicalVolumeRadius(par[4]);
			else m_THMFit->model()->SetStrangenessCanonicalVolumeRadius(par[3]);

			m_THMFit->model()->SetGammaq(par[5]);

			if (m_THMFit->Parameters().muQ.toFit == false)
				m_THMFit->model()->SetElectricChemicalPotential(-par[1] / 50.);
			else
				m_THMFit->model()->SetElectricChemicalPotential(par[6]);
			m_THMFit->model()->ConstrainMuQ(!m_THMFit->Parameters().muQ.toFit);

			if (m_THMFit->Parameters().muS.toFit == false)
				m_THMFit->model()->SetStrangenessChemicalPotential(par[1] / 5.);
			else
				m_THMFit->model()->SetStrangenessChemicalPotential(par[7]);
			m_THMFit->model()->ConstrainMuS(!m_THMFit->Parameters().muS.toFit);

			m_THMFit->model()->SetParameters(m_THMFit->model()->Parameters());

			m_THMFit->model()->SetQoverB(m_THMFit->QoverB());
			m_THMFit->model()->FixParameters();

			m_THMFit->model()->CalculateDensities();

			double Npart = m_THMFit->model()->CalculateBaryonDensity() * m_THMFit->model()->Parameters().V;

			// Ratios first
			for (int i = 0; i < m_THMFit->FittedQuantities().size(); ++i) {
				if (m_THMFit->FittedQuantities()[i].toFit && m_THMFit->FittedQuantities()[i].type == FittedQuantity::Ratio) {
					const ExperimentRatio &ratio = m_THMFit->FittedQuantities()[i].ratio;
					double dens1 = m_THMFit->GetDensity(ratio.PDGID1, ratio.fFeedDown1);
					double dens2 = m_THMFit->GetDensity(ratio.PDGID2, ratio.fFeedDown2);
					double ModelRatio = dens1 / dens2;
					chi2 += (ModelRatio - ratio.fValue) * (ModelRatio - ratio.fValue) / ratio.fError / ratio.fError;
				}
			}

			// Yields second
			for (int i = 0; i < m_THMFit->FittedQuantities().size(); ++i) {
				if (m_THMFit->FittedQuantities()[i].toFit && m_THMFit->FittedQuantities()[i].type == FittedQuantity::Multiplicity) {
					const ExperimentMultiplicity &multiplicity = m_THMFit->FittedQuantities()[i].mult;
					double dens = m_THMFit->GetDensity(multiplicity.fPDGID, multiplicity.fFeedDown);
					double ModelMult = dens * m_THMFit->model()->Parameters().V;
					chi2 += (ModelMult - multiplicity.fValue) * (ModelMult - multiplicity.fValue) / multiplicity.fError / multiplicity.fError;
				}
			}

			if (m_verbose) {
				printf("%15d", migraditer);
				printf("%15lf", chi2);
				if (m_THMFit->Parameters().T.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().T);
				if (m_THMFit->Parameters().muB.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().muB);
				if (m_THMFit->Parameters().muQ.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().muQ);
				if (m_THMFit->Parameters().muS.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().muS);
				if (m_THMFit->Parameters().muC.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().muC);
				if (m_THMFit->Parameters().R.toFit)
					printf("%15lf", par[3]);
				if (m_THMFit->Parameters().Rc.toFit)
					printf("%15lf", par[4]);
				if (m_THMFit->Parameters().gammaq.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().gammaq);
				if (m_THMFit->Parameters().gammaS.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().gammaS);
				if (m_THMFit->Parameters().gammaC.toFit)
					printf("%15lf", m_THMFit->model()->Parameters().gammaC);
				printf("\n");

				if (m_THMFit->model()->Ensemble() == ThermalModelBase::CE)
					printf("B = %10.5lf\tQ = %10.5lf\tS = %10.5lf\tC = %10.5lf\n", 
						m_THMFit->model()->CalculateBaryonDensity() * m_THMFit->model()->Parameters().V,
						m_THMFit->model()->CalculateChargeDensity() * m_THMFit->model()->Parameters().V, 
						m_THMFit->model()->CalculateStrangenessDensity() * m_THMFit->model()->Parameters().V,
						m_THMFit->model()->CalculateCharmDensity() * m_THMFit->model()->Parameters().V);
			}

			m_THMFit->Chi2() = chi2;
			if (m_THMFit->model()->Ensemble() == ThermalModelBase::CE) {
				m_THMFit->BT()   = m_THMFit->model()->CalculateBaryonDensity()      * m_THMFit->model()->Parameters().V;
				m_THMFit->QT()   = m_THMFit->model()->CalculateChargeDensity()      * m_THMFit->model()->Parameters().V;
				m_THMFit->ST()   = m_THMFit->model()->CalculateStrangenessDensity() * m_THMFit->model()->Parameters().V;
				m_THMFit->CT()   = m_THMFit->model()->CalculateCharmDensity()       * m_THMFit->model()->Parameters().V;
			}

			if (chi2!=chi2) {
				chi2 = 1.e12;
				printf("**WARNING** Error calculating chi2\n");
			}

			return chi2;
		}

		double Up() const {return 1.;}

	private:
		ThermalModelFit *m_THMFit;
		int		m_iter;
		bool 	m_verbose;
	};
}

using namespace ThermalModelFitNamespace;

#endif

ThermalModelFit::ThermalModelFit(ThermalModelBase *model_):
	m_model(model_), m_Parameters(model_->Parameters()), m_QBgoal(0.4)
{
}


ThermalModelFit::~ThermalModelFit(void)
{
}

ThermalModelFitParameters ThermalModelFit::PerformFit(bool verbose, bool AsymmErrors) {
#ifdef USE_MINUIT
	m_Parameters.Rc.value = m_Parameters.R.value;
	m_Parameters.Rc.xmin  = m_Parameters.R.xmin;
	m_Parameters.Rc.xmax  = m_Parameters.R.xmax;

	m_Iters = 0;
	FitFCN mfunc(this, verbose);
	std::vector<double> params(8, 0.);
	params[0] = m_Parameters.T.value;
	params[1] = m_Parameters.muB.value;
	params[2] = m_Parameters.gammaS.value;
	params[3] = m_Parameters.R.value;
	params[4] = m_Parameters.Rc.value;
	params[5] = m_Parameters.gammaq.value;
	params[6] = m_Parameters.muQ.value;
	params[7] = m_Parameters.muS.value;

	MnUserParameters upar;
	upar.Add("T", m_Parameters.T.value, m_Parameters.T.error, m_Parameters.T.xmin, m_Parameters.T.xmax);
	upar.Add("muB", m_Parameters.muB.value, m_Parameters.muB.error, m_Parameters.muB.xmin, m_Parameters.muB.xmax);
	upar.Add("gammaS", m_Parameters.gammaS.value, m_Parameters.gammaS.error, m_Parameters.gammaS.xmin, m_Parameters.gammaS.xmax);
	upar.Add("R", m_Parameters.R.value, m_Parameters.R.error, m_Parameters.R.xmin, m_Parameters.R.xmax);
	upar.Add("Rc", m_Parameters.Rc.value, m_Parameters.Rc.error, m_Parameters.Rc.xmin, m_Parameters.Rc.xmax);
	upar.Add("gammaq", m_Parameters.gammaq.value, m_Parameters.gammaq.error, m_Parameters.gammaq.xmin, m_Parameters.gammaq.xmax);
	upar.Add("muQ", m_Parameters.muQ.value, m_Parameters.muQ.error, m_Parameters.muQ.xmin, m_Parameters.muQ.xmax);
	upar.Add("muS", m_Parameters.muS.value, m_Parameters.muS.error, m_Parameters.muS.xmin, m_Parameters.muS.xmax);

	int nparams = 8;
	if (m_model->Ensemble() == ThermalModelBase::CE) {
		m_Parameters.SetParameterFitFlag("muB", false);
		m_Parameters.R.xmax = 5.;
	}
	if (!m_Parameters.T.toFit) { upar.Fix("T"); nparams--; }
	if (!m_Parameters.muB.toFit) { upar.Fix("muB"); nparams--; }
	if (!m_Parameters.gammaq.toFit) { upar.Fix("gammaq"); nparams--; }
	if (!m_Parameters.gammaS.toFit) { upar.Fix("gammaS"); nparams--; }
	if (!m_Parameters.R.toFit || 
		(m_Multiplicities.size()==0 && m_model->Ensemble() == ThermalModelBase::GCE)) { m_Parameters.R.toFit = false; upar.Fix("R"); nparams--; }
	if (!m_Parameters.Rc.toFit || 
		m_model->Ensemble() != ThermalModelBase::SCE)
		{ 
			m_Parameters.Rc.toFit = false; 
			upar.Fix("Rc"); 
			nparams--; 
		}
	if (!m_Parameters.muQ.toFit) { upar.Fix("muQ"); nparams--; }
	if (!m_Parameters.muS.toFit) { upar.Fix("muS"); nparams--; }

	m_Ndf = GetNdf();

	bool repeat = 0;

	ThermalModelFitParameters ret;

	if (nparams>0) {

		if (verbose) {
			printf("Starting a thermal fit...\n\n");
			printf("%15s", "Iteration");
			printf("%15s", "chi2");
			if (m_Parameters.T.toFit)
				printf("%15s", "T [GeV]");
			if (m_Parameters.muB.toFit)
				printf("%15s", "muB [GeV]");
			if (m_Parameters.muQ.toFit)
				printf("%15s", "muQ [GeV]");
			if (m_Parameters.muS.toFit)
				printf("%15s", "muS [GeV]");
			if (m_Parameters.muC.toFit)
				printf("%15s", "muC [GeV]");
			if (m_Parameters.R.toFit)
				printf("%15s", "R [fm]");
			if (m_Parameters.Rc.toFit)
				printf("%15s", "Rc [fm]");
			if (m_Parameters.gammaq.toFit)
				printf("%15s", "gammaq");
			if (m_Parameters.gammaS.toFit)
				printf("%15s", "gammaS");
			if (m_Parameters.gammaC.toFit)
				printf("%15s", "gammaC");
			printf("\n");
		}

		MnMigrad migrad(mfunc, upar);

		FunctionMinimum min = migrad();

		if (verbose)
			printf("\nMinimum found! Now calculating the error matrix...\n\n");

		MnHesse hess;
		hess(mfunc, min);

		ret = m_Parameters;

		if (AsymmErrors) {
			MnMinos mino(mfunc, min);
			std::pair<double, double> errs;
			if (m_Parameters.T.toFit) { errs = mino(0); ret.T.errm = abs(errs.first); ret.T.errp = abs(errs.second); }
			if (m_Parameters.muB.toFit) { errs = mino(1); ret.muB.errm = abs(errs.first); ret.muB.errp = abs(errs.second); }
			if (m_Parameters.gammaS.toFit) { errs = mino(2); ret.gammaS.errm = abs(errs.first); ret.gammaS.errp = abs(errs.second); }
			if (m_Parameters.R.toFit) { errs = mino(3); ret.R.errm = abs(errs.first); ret.R.errp = abs(errs.second); }
			if (m_Parameters.Rc.toFit) { errs = mino(4); ret.Rc.errm = abs(errs.first); ret.Rc.errp = abs(errs.second); }
			if (m_Parameters.gammaq.toFit) { errs = mino(5); ret.gammaq.errm = abs(errs.first); ret.gammaq.errp = abs(errs.second); }
		}

		ret.T.value = (min.UserParameters()).Params()[0];
		ret.T.error = (min.UserParameters()).Errors()[0];
		ret.muB.value = (min.UserParameters()).Params()[1];
		ret.muB.error = (min.UserParameters()).Errors()[1];
		ret.gammaS.value = (min.UserParameters()).Params()[2];
		ret.gammaS.error = (min.UserParameters()).Errors()[2];
		ret.R.value = (min.UserParameters()).Params()[3];
		ret.R.error = (min.UserParameters()).Errors()[3];
		ret.Rc.value = (min.UserParameters()).Params()[4];
		ret.Rc.error = (min.UserParameters()).Errors()[4];
		ret.gammaq.value = (min.UserParameters()).Params()[5];
		ret.gammaq.error = (min.UserParameters()).Errors()[5];
		ret.muQ.value = (min.UserParameters()).Params()[6];
		ret.muQ.error = (min.UserParameters()).Errors()[6];
		ret.muS.value = (min.UserParameters()).Params()[7];
		ret.muS.error = (min.UserParameters()).Errors()[7];

		if (!m_Parameters.Rc.toFit) {
			ret.Rc = ret.R;
			ret.Rc.name = "Rc";
		}

		if (repeat) {
			m_model->SetUseWidth(true);

			upar.SetValue("T", (min.UserParameters()).Params()[0]);
			upar.SetValue("muB", (min.UserParameters()).Params()[1]);
			upar.SetValue("gammaS", (min.UserParameters()).Params()[2]);
			upar.SetValue("R", (min.UserParameters()).Params()[3]);
			upar.SetValue("Rc", (min.UserParameters()).Params()[4]);
			upar.SetValue("gammaq", (min.UserParameters()).Params()[5]);
			upar.SetValue("muQ", (min.UserParameters()).Params()[6]);
			upar.SetValue("muS", (min.UserParameters()).Params()[7]);

			MnMigrad migradd(mfunc, upar);
			min = migradd();

			ret = m_Parameters;
			ret.T.value = (min.UserParameters()).Params()[0];
			ret.T.error = (min.UserParameters()).Errors()[0];
			ret.muB.value = (min.UserParameters()).Params()[1];
			ret.muB.error = (min.UserParameters()).Errors()[1];
			ret.gammaS.value = (min.UserParameters()).Params()[2];
			ret.gammaS.error = (min.UserParameters()).Errors()[2];
			ret.R.value = (min.UserParameters()).Params()[3];
			ret.R.error = (min.UserParameters()).Errors()[3];
			ret.Rc.value = (min.UserParameters()).Params()[4];
			ret.Rc.error = (min.UserParameters()).Errors()[4];
			ret.gammaq.value = (min.UserParameters()).Params()[5];
			ret.gammaq.error = (min.UserParameters()).Errors()[5];
			ret.muQ.value = (min.UserParameters()).Params()[6];
			ret.muQ.error = (min.UserParameters()).Errors()[6];
			ret.muS.value = (min.UserParameters()).Params()[7];
			ret.muS.error = (min.UserParameters()).Errors()[7];
		}
	}
	else {
		ret = m_Parameters;

		ret.T.value = upar.Params()[0];
		ret.T.error = upar.Errors()[0];
		ret.muB.value = upar.Params()[1];
		ret.muB.error = upar.Errors()[1];
		ret.gammaS.value = upar.Params()[2];
		ret.gammaS.error = upar.Errors()[2];
		ret.R.value = upar.Params()[3];
		ret.R.error = upar.Errors()[3];
		ret.Rc.value = upar.Params()[4];
		ret.Rc.error = upar.Errors()[4];
		ret.gammaq.value = upar.Params()[5];
		ret.gammaq.error = upar.Errors()[5];
		ret.muQ.value = upar.Params()[6];
		ret.muQ.error = upar.Errors()[6];
		ret.muS.value = upar.Params()[7];
		ret.muS.error = upar.Errors()[7];
	}

	ret.muC.value = m_model->Parameters().muC;
	ret.muC.error = 0.;
	ret.gammaC.value = m_model->Parameters().gammaC;
	ret.gammaC.error = 0.;

	ThermalModelParameters parames = ret.GetThermalModelParameters();

	parames.B = m_model->Parameters().B;
	parames.S = m_model->Parameters().S;
	parames.Q = m_model->Parameters().Q;
	m_model->SetParameters(parames);
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	if (!ret.muQ.toFit) {
		ret.muQ.value = m_model->Parameters().muQ;
		ret.muQ.error = 0.;
	}
	if (!ret.muS.toFit) {
		ret.muS.value = m_model->Parameters().muS;
		ret.muS.error = 0.;
	}
	m_Parameters = ret;

	params[0] = ret.T.value;
	params[1] = ret.muB.value;
	params[2] = ret.gammaS.value;
	params[3] = ret.R.value;
	params[4] = ret.Rc.value;
	params[5] = ret.gammaq.value;
	params[6] = ret.muQ.value;
	params[7] = ret.muS.value;


	ret.chi2    = mfunc(params);
	int ndf = 0;
	for (int i = 0; i < m_Quantities.size(); ++i)
		if (m_Quantities[i].toFit) ndf++;
	ndf = ndf - nparams;

	ret.chi2ndf = ret.chi2 / ndf;
	ret.ndf = ndf;
	m_Parameters = ret;

	m_ExtendedParameters = ThermalModelFitParametersExtended(m_model);

	
	if (verbose)
		printf("Thermal fit finished\n\n");
	return ret;
#else
	printf("**ERROR** Cannot fit without MINUIT2 library!\n");
	exit(1);
#endif
}

void ThermalModelFit::PrintRatios() {
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();

	for (int i = 0; i < m_Quantities.size(); ++i) {
		if (m_Quantities[i].type == FittedQuantity::Ratio) {
			const ExperimentRatio &ratio = m_Quantities[i].ratio;
			int ind1 = m_model->TPS()->PdgToId(ratio.PDGID1);
			int ind2 = m_model->TPS()->PdgToId(ratio.PDGID2);
			double dens1 = GetDensity(ratio.PDGID1, 1);
			double dens2 = GetDensity(ratio.PDGID2, 1);
			std::cout << m_model->TPS()->Particles()[ind1].Name() << "/" << m_model->TPS()->Particles()[ind2].Name() << " = " <<
				dens1 / dens2 << " " << ratio.fValue << " " << ratio.fError << "\n";
		}
	}
}

void ThermalModelFit::PrintParameters() {
	printf("%20s\n", "Fit parameters");
	for(int i=0;i<6;++i) {
		FitParameter param = m_Parameters.GetParameter(i);
		if (param.toFit) {
			printf("%10s = %10lf %2s %lf\n", param.name.c_str(), param.value, "+-",  param.error);
		}
	}
	if (m_model->Ensemble() == ThermalModelBase::CE) {
		printf("%10s = %10lf\n", "B", m_model->CalculateBaryonDensity() * m_model->Volume());
		printf("%10s = %10lf\n", "S", m_model->CalculateStrangenessDensity() * m_model->Volume());
		printf("%10s = %10lf\n", "Q", m_model->CalculateChargeDensity() * m_model->Volume());
	}
	printf("\n");
}


void ThermalModelFit::PrintMultiplicities() {
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();

	for (int i = 0; i < m_Quantities.size(); ++i) {
		if (m_Quantities[i].type == FittedQuantity::Multiplicity) {
			const ExperimentMultiplicity &mult = m_Quantities[i].mult;

			double dens1 = GetDensity(mult.fPDGID, mult.fFeedDown);

			std::string tname = m_model->TPS()->GetNameFromPDG(mult.fPDGID);

			if (mult.fPDGID == 1) 
				printf("%10s\t%11s\t%6lf %2s %lf\n", "Npart", "Experiment:", 
					mult.fValue, "+-", mult.fError);
			else if (mult.fPDGID == 33340) 
				printf("%10s\t%11s\t%6lf %2s %lf\n", (m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name()).c_str(), "Experiment:", 
					mult.fValue, "+-", mult.fError);
			else 
				printf("%10s\t%11s\t%6lf %2s %lf\n", tname.c_str(), "Experiment:", mult.fValue, "+-", mult.fError);
			printf("%10s\t%11s\t%6lf %2s %lf\n", "", "Model:", dens1 * m_model->Parameters().V, "+-", 0.);
			printf("%10s\t%11s\t%6lf\n", "", "Deviation:", (dens1 * m_model->Parameters().V - mult.fValue) / mult.fError);
			printf("\n");

		}
	}

}

void ThermalModelFit::PrintYields() {
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();

	for (int i = 0; i < m_Quantities.size(); ++i) {
		if (m_Quantities[i].type == FittedQuantity::Multiplicity) {
			const ExperimentMultiplicity &mult = m_Quantities[i].mult;

			double dens1 = GetDensity(mult.fPDGID, mult.fFeedDown);

			std::string tname = m_model->TPS()->GetNameFromPDG(mult.fPDGID);

			if (mult.fPDGID == 1)
				printf("%10s\t%11s\t%6lf %2s %lf\n", "Npart", "Experiment:",
					mult.fValue, "+-", mult.fError);
			else if (mult.fPDGID == 33340)
				printf("%10s\t%11s\t%6lf %2s %lf\n", (m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name()).c_str(), "Experiment:",
					mult.fValue, "+-", mult.fError);
			else
				printf("%10s\t%11s\t%6lf %2s %lf\n", tname.c_str(), "Experiment:", mult.fValue, "+-", mult.fError);
			printf("%10s\t%11s\t%6lf %2s %lf\n", "", "Model:", dens1 * m_model->Parameters().V, "+-", 0.);
			printf("%10s\t%11s\t%6lf\n", "", "Deviation:", (dens1 * m_model->Parameters().V - mult.fValue) / mult.fError);
			printf("\n");

		}
	}

	for (int i = 0; i < m_Quantities.size(); ++i) {
		if (m_Quantities[i].type == FittedQuantity::Ratio) {
			const ExperimentRatio &ratio = m_Quantities[i].ratio;
			double dens1 = GetDensity(ratio.PDGID1, ratio.fFeedDown1);
			double dens2 = GetDensity(ratio.PDGID2, ratio.fFeedDown2);
			std::string name1 = m_model->TPS()->GetNameFromPDG(ratio.PDGID1);
			std::string name2 = m_model->TPS()->GetNameFromPDG(ratio.PDGID2);
			if (ratio.PDGID1 == 1)
				name1 = "Npart";
			if (ratio.PDGID2 == 1)
				name2 = "Npart";
			if (ratio.PDGID1 == 33340)
				name1 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
			if (ratio.PDGID2 == 33340)
				name2 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
			printf("%10s\t%11s\t%6lf %2s %lf\n", (std::string(name1 + "/" + name2)).c_str(), "Experiment:", 
				ratio.fValue, "+-", ratio.fError);
			printf("%10s\t%11s\t%6lf %2s %lf\n", "", "Model:", dens1 / dens2, "+-", 0.);
			printf("%10s\t%11s\t%6lf\n", "", "Deviation:", (dens1 / dens2 - ratio.fValue) / ratio.fError);
			printf("\n");
		}
	}
}

void ThermalModelFit::PrintYieldsTable(std::string filename) {
	FILE *f = fopen(filename.c_str(), "w");

	fprintf(f, "%15s\t%25s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s\n", 
		"N", "Name", "Data", "Error", "Fit", "Deviation", "Deviation2", 
		"Residual", "ResidualError", "Data/Model", "Data/ModelError");

	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();
	for(int i=0;i<m_Multiplicities.size();++i) {
		double dens1 = GetDensity(m_Multiplicities[i].fPDGID, m_Multiplicities[i].fFeedDown);

		std::string tname =  m_model->TPS()->GetNameFromPDG(m_Multiplicities[i].fPDGID);
		if (m_Multiplicities[i].fPDGID==1) tname =  "Npart";
		else if (m_Multiplicities[i].fPDGID==33340) tname =  m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
		fprintf(f, "%15d\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1, 
			tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, 
			(m_Multiplicities[i].fValue-dens1 * m_model->Parameters().V)/m_Multiplicities[i].fError,
			-(m_Multiplicities[i].fValue-dens1 * m_model->Parameters().V)/m_Multiplicities[i].fError,
			(dens1 * m_model->Parameters().V-m_Multiplicities[i].fValue)/(dens1 * m_model->Parameters().V),
			m_Multiplicities[i].fError/(dens1 * m_model->Parameters().V),
			m_Multiplicities[i].fValue/(dens1 * m_model->Parameters().V),
			m_Multiplicities[i].fError/(dens1 * m_model->Parameters().V));
	}
	for(int i=0;i<m_Ratios.size();++i) {
		double dens1 = GetDensity(m_Ratios[i].PDGID1, m_Ratios[i].fFeedDown1);
		double dens2 = GetDensity(m_Ratios[i].PDGID2, m_Ratios[i].fFeedDown2);
		std::string name1 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID1);
		std::string name2 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID2);
		if (m_Ratios[i].PDGID1==1) name1 = "Npart";
		if (m_Ratios[i].PDGID2==1) name2 = "Npart";
		if (m_Ratios[i].PDGID1==33340) name1 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
		if (m_Ratios[i].PDGID2==33340) name2 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();

		fprintf(f, "%15d\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1, 
			(std::string(name1 + "/" + name2)).c_str(),  m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2, 
			(m_Ratios[i].fValue - dens1 / dens2)/m_Ratios[i].fError,
			-(m_Ratios[i].fValue - dens1 / dens2)/m_Ratios[i].fError,
			(dens1 / dens2-m_Ratios[i].fValue)/(dens1 / dens2),
			m_Ratios[i].fError/(dens1 / dens2),
			m_Ratios[i].fValue/(dens1 / dens2),
			m_Ratios[i].fError/(dens1 / dens2));
	}

	fclose(f);
}

void ThermalModelFit::PrintYieldsTable2(std::string filename) {
	FILE *f = fopen(filename.c_str(), "w");

	fprintf(f, "%15s\t%25s\t%15s\t%15s\t%15s\t%15s\n", "N", "Name", "Data", "Error", "Fit", "Deviation");
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();
	for(int i=0;i<m_Multiplicities.size();++i) {

		double dens1 = GetDensity(m_Multiplicities[i].fPDGID, m_Multiplicities[i].fFeedDown);
		std::string tname =  m_model->TPS()->GetNameFromPDG(m_Multiplicities[i].fPDGID);
		if (m_Multiplicities[i].fPDGID==1) tname =  "Npart";
		else if (m_Multiplicities[i].fPDGID==33340) tname =  m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
		fprintf(f, "%15lf\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1 - 0.3, tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, (m_Multiplicities[i].fValue-dens1 * m_model->Parameters().V)/m_Multiplicities[i].fError);
		fprintf(f, "%15lf\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1 + 0.3, tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, (m_Multiplicities[i].fValue-dens1 * m_model->Parameters().V)/m_Multiplicities[i].fError);
	}
	for(int i=0;i<m_Ratios.size();++i) {
		double dens1 = GetDensity(m_Ratios[i].PDGID1, m_Ratios[i].fFeedDown1);
		double dens2 = GetDensity(m_Ratios[i].PDGID2, m_Ratios[i].fFeedDown2);

		std::string name1 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID1);
		std::string name2 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID2);
		if (m_Ratios[i].PDGID1==1) name1 = "Npart";
		if (m_Ratios[i].PDGID2==1) name2 = "Npart";
		if (m_Ratios[i].PDGID1==33340) name1 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
		if (m_Ratios[i].PDGID2==33340) name2 = m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();

		fprintf(f, "%15lf\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1 - 0.3, (std::string(name1 + "/" + name2)).c_str(),  m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2, (m_Ratios[i].fValue - dens1 / dens2)/m_Ratios[i].fError);
		fprintf(f, "%15lf\t%25s\t%15lf\t%15lf\t%15lf\t%15lf\n", i+1 + 0.3, (std::string(name1 + "/" + name2)).c_str(),  m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2, (m_Ratios[i].fValue - dens1 / dens2)/m_Ratios[i].fError);
	}

	fclose(f);
}

void ThermalModelFit::PrintYieldsLatex(std::string filename, std::string name) {
	FILE *f = fopen(filename.c_str(), "w");
	fprintf(f, "\\begin{tabular}{|c|c|c|c|}\n");
	fprintf(f, "\\hline\n");
	fprintf(f, "\\multicolumn{4}{|c|}{%s} \\\\\n", name.c_str());
	fprintf(f, "\\hline\n");
	fprintf(f, "Yield & Measurement & Fit & Deviation \\\\\n");
	fprintf(f, "\\hline\n");
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();
	for(int i=0;i<m_Multiplicities.size();++i) {
		double dens1 = GetDensity(m_Multiplicities[i].fPDGID, m_Multiplicities[i].fFeedDown);

		std::string tname =  m_model->TPS()->GetNameFromPDG(m_Multiplicities[i].fPDGID);

		if (m_Multiplicities[i].fPDGID==1) tname =  "Npart";
		else if (m_Multiplicities[i].fPDGID==33340) tname =  m_model->TPS()->ParticleByPDG(3334).Name() + " + " + m_model->TPS()->ParticleByPDG(-3334).Name();
		fprintf(f, "$%s$ & $%.4lf \\pm %.4lf$ & $%.4lf$ & $%.4lf$ \\\\\n", tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, (dens1 * m_model->Parameters().V-m_Multiplicities[i].fValue)/m_Multiplicities[i].fError);
	}
	for(int i=0;i<m_Ratios.size();++i) {
		int ind1 = m_model->TPS()->PdgToId(m_Ratios[i].PDGID1);
		int ind2 = m_model->TPS()->PdgToId(m_Ratios[i].PDGID2);
		double dens1 = GetDensity(m_Ratios[i].PDGID1, m_Ratios[i].fFeedDown1);
		double dens2 = GetDensity(m_Ratios[i].PDGID2, m_Ratios[i].fFeedDown2);

		std::string name1 = m_model->TPS()->Particles()[ind1].Name();
		std::string name2 = m_model->TPS()->Particles()[ind2].Name();
		if (m_Ratios[i].PDGID1==1) name1 = "Npart";
		if (m_Ratios[i].PDGID2==1) name2 = "Npart";
		if (m_Ratios[i].PDGID1==33340) name1 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
		if (m_Ratios[i].PDGID2==33340) name2 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
		fprintf(f, "$%s$ & $%.4lf \\pm %.4lf$ & $%.4lf$ & $%.4lf$ \\\\\n", (std::string(name1 + "/" + name2)).c_str(), m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2 , (dens1 / dens2 - m_Ratios[i].fValue)/m_Ratios[i].fError);
	}
	fprintf(f, "\\hline\n");
	fprintf(f, "\\end{tabular}\n");

	fprintf(f, "\\begin{tabular}{|c|c|}\n");
	fprintf(f, "\\hline\n");
	fprintf(f, "\\multicolumn{2}{|c|}{%s} \\\\\n", name.c_str());
	fprintf(f, "\\hline\n");
	fprintf(f, "Parameter & Fit result \\\\\n");
	fprintf(f, "\\hline\n");
	for(int i=0;i<6;++i) {
		FitParameter param = m_Parameters.GetParameter(i);
		if (param.toFit) {
			double tval = param.value, terr = param.error;
			if (param.name=="T" || param.name.substr(0,2)=="mu") { tval *= 1000.; terr *= 1000.; }
			fprintf(f, "$%s$ & $%.2lf \\pm %.2lf$ \\\\\n", param.name.c_str(), tval, terr);
		}
	}
	fprintf(f, "$%s$ & $%.2lf/%d$ \\\\\n", "\\chi^2/N_{\\rm df}", m_Parameters.chi2ndf * GetNdf(), GetNdf());
	fprintf(f, "\\hline\n");
	fprintf(f, "\\end{tabular}\n");

	fclose(f);
}

void ThermalModelFit::PrintYieldsLatexAll(std::string filename, std::string name, bool asymm) {
	FILE *f = fopen(filename.c_str(), "w");
	fprintf(f, "\\begin{tabular}{|c|c|c|c|}\n");
	fprintf(f, "\\hline\n");
	fprintf(f, "\\multicolumn{4}{|c|}{%s} \\\\\n", name.c_str());
	fprintf(f, "\\hline\n");
	fprintf(f, "Yield & Measurement & Fit \\\\\n");
	fprintf(f, "\\hline\n");
	m_model->SetParameters(m_Parameters.GetThermalModelParameters());
	m_model->SetQoverB(m_QBgoal);
	m_model->FixParameters();
	m_model->CalculateDensities();

	std::vector<std::string> prt(0);
	std::vector<int> pdgs(0);
	std::vector<int> fl(m_Multiplicities.size(), 0);
	prt.push_back("\\pi^+");   pdgs.push_back(211);
	prt.push_back("\\pi^-");   pdgs.push_back(-211);
	prt.push_back("K^+");      pdgs.push_back(321);
	prt.push_back("K^-");      pdgs.push_back(-321);
	prt.push_back("p");        pdgs.push_back(2212);
	prt.push_back("\\bar{p}"); pdgs.push_back(-2212);
	prt.push_back("\\Lambda"); pdgs.push_back(3122);
	prt.push_back("\\bar{\\Lambda}"); pdgs.push_back(-3122);
	prt.push_back("\\Sigma^+");   pdgs.push_back(3222);
	prt.push_back("\\bar{\\Sigma}^+"); pdgs.push_back(-3222);
	prt.push_back("\\Sigma^-");   pdgs.push_back(3112);
	prt.push_back("\\bar{\\Sigma}^-"); pdgs.push_back(-3112);
	prt.push_back("\\Xi^0");   pdgs.push_back(3322);
	prt.push_back("\\bar{\\Xi}^0"); pdgs.push_back(-3322);
	prt.push_back("\\Xi^-");   pdgs.push_back(3312);
	prt.push_back("\\bar{\\Xi}^-"); pdgs.push_back(-3312);
	prt.push_back("\\Omega");  pdgs.push_back(3334);
	prt.push_back("\\bar{\\Omega}"); pdgs.push_back(-3334);
	prt.push_back("\\pi^0");   pdgs.push_back(111);
	prt.push_back("K^0_S");    pdgs.push_back(310);
	prt.push_back("\\eta");    pdgs.push_back(221);
	prt.push_back("\\omega");  pdgs.push_back(223);
	prt.push_back("K^{*+}");      pdgs.push_back(323);
	prt.push_back("K^{*-}");      pdgs.push_back(-323);
	prt.push_back("K^{*0}");      pdgs.push_back(313);
	prt.push_back("\\bar{K^{*0}}");      pdgs.push_back(-313);
	//prt.push_back("K^*(892)"); pdgs.push_back(323);
	prt.push_back("\\rho^+");		  pdgs.push_back(213);
	prt.push_back("\\rho^-");		  pdgs.push_back(-213);
	prt.push_back("\\rho^0");		  pdgs.push_back(113);
	prt.push_back("\\eta'");    pdgs.push_back(331);
	prt.push_back("\\phi");    pdgs.push_back(333);
	prt.push_back("\\Lambda(1520)");  pdgs.push_back(3124);



		prt.push_back("d"); pdgs.push_back(1000010020);
		prt.push_back("^3He"); pdgs.push_back(1000020030);
		prt.push_back("^3_{\\Lambda}H"); pdgs.push_back(1010010030);
		prt.push_back("D^+"); pdgs.push_back(411);
		prt.push_back("D^-"); pdgs.push_back(-411);
		prt.push_back("D^0"); pdgs.push_back(421);
		prt.push_back("\\bar{D}^0"); pdgs.push_back(-421);

	for(int i=0;i<prt.size();++i) {
		bool isexp = false;
		int tj = -1;
		for(int j=0;j<m_Multiplicities.size();++j)
			if (pdgs[i]==m_Multiplicities[j].fPDGID) { isexp = true; fl[j] = 1; tj = j; break; }

			if (m_model->TPS()->PdgToId(pdgs[i]) == -1) continue;

			double dens1 = 0.;
			dens1 = GetDensity(pdgs[i], 1);

			if (isexp) dens1 = GetDensity(pdgs[i], m_Multiplicities[tj].fFeedDown);

			std::string tname = prt[i];

			if (!isexp) {
				if (dens1 * m_model->Parameters().V>1.e-3) fprintf(f, "$%s$ &  & $%.3g$ \\\\\n", tname.c_str(), dens1 * m_model->Parameters().V);
				else {
					char tnum[400];
					sprintf(tnum, "%.2E", dens1 * m_model->Parameters().V);
					double val = 0.;
					int step = 0;
					char valst[6];
					char stepst[6];
					for(int i=0;i<4;++i) valst[i] = tnum[i];
					valst[4] = 0;
					for(int i=0;i<4;++i) stepst[i] = tnum[5+i];
					stepst[4] = 0;
					val = atof(valst);
					step = atoi(stepst);
					fprintf(f, "$%s$ & & $%.2lf \\cdot 10^{%d}$ \\\\\n", tname.c_str(), val, step);
				}
			}
			else {
				if (m_Multiplicities[tj].fValue>0.999) fprintf(f, "$%s$ & $%.4g \\pm %.4g$ & $%.4g$  \\\\\n", tname.c_str(), m_Multiplicities[tj].fValue, m_Multiplicities[tj].fError, dens1 * m_model->Parameters().V);//, (dens1 * m_model->Parameters().V-m_Multiplicities[tj].fValue)/m_Multiplicities[tj].fError);
				else if (dens1 * m_model->Parameters().V>1.e-3) fprintf(f, "$%s$ & $%.3g \\pm %.3g$ & $%.3g$ \\\\\n", tname.c_str(), m_Multiplicities[tj].fValue, m_Multiplicities[tj].fError, dens1 * m_model->Parameters().V);//, (dens1 * m_model->Parameters().V-m_Multiplicities[tj].fValue)/m_Multiplicities[tj].fError);
				else fprintf(f, "$%s$ & $%.6g \\pm %.6g$ & $%.3g$ \\\\\n", tname.c_str(), m_Multiplicities[tj].fValue, m_Multiplicities[tj].fError, dens1 * m_model->Parameters().V);//, (dens1 * m_model->Parameters().V-m_Multiplicities[tj].fValue)/m_Multiplicities[tj].fError);
			}
	}

	for(int i=0;i<m_Multiplicities.size();++i) 
		if (!fl[i]) {
			double dens1 = GetDensity(m_Multiplicities[i].fPDGID, m_Multiplicities[i].fFeedDown);

			std::string tname =  m_model->TPS()->GetNameFromPDG(m_Multiplicities[i].fPDGID);
			if (m_Multiplicities[i].fPDGID==1) tname =  "Npart";
			else if (m_Multiplicities[i].fPDGID==33340) tname =  m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
			fprintf(f, "$%s$ & $%.4lf \\pm %.4lf$ & $%.4lf$ & $%.4lf$ \\\\\n", tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, (dens1 * m_model->Parameters().V-m_Multiplicities[i].fValue)/m_Multiplicities[i].fError);
		}
		for(int i=0;i<m_Ratios.size();++i) {
			double dens1 = GetDensity(m_Ratios[i].PDGID1, m_Ratios[i].fFeedDown1);
			double dens2 = GetDensity(m_Ratios[i].PDGID2, m_Ratios[i].fFeedDown2);

			std::string name1 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID1);
			std::string name2 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID2);
			if (m_Ratios[i].PDGID1==1) name1 = "Npart";
			if (m_Ratios[i].PDGID2==1) name2 = "Npart";
			if (m_Ratios[i].PDGID1==33340) name1 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
			if (m_Ratios[i].PDGID2==33340) name2 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
			fprintf(f, "$%s$ & $%.4lf \\pm %.4lf$ & $%.4lf$ & $%.4lf$ \\\\\n", (std::string(name1 + "/" + name2)).c_str(), m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2 , (dens1 / dens2 - m_Ratios[i].fValue)/m_Ratios[i].fError);
		}
		fprintf(f, "\\hline\n");
		fprintf(f, "\\end{tabular}\n");

		fprintf(f, "\\begin{tabular}{|c|c|}\n");
		fprintf(f, "\\hline\n");
		fprintf(f, "\\multicolumn{2}{|c|}{%s} \\\\\n", name.c_str());
		fprintf(f, "\\hline\n");
		fprintf(f, "Parameter & Fit result \\\\\n");
		fprintf(f, "\\hline\n");
		for(int i=0;i<9;++i) {
			FitParameter param = m_Parameters.GetParameter(i);
			if (param.toFit) {
				double tval = param.value, terr = param.error, terrp = param.errp, terrm = param.errm;
				if (param.name=="T" || param.name.substr(0,2)=="mu") { tval *= 1000.; terr *= 1000.; terrp *= 1000.; terrm *= 1000.; }
				if (!asymm) fprintf(f, "$%s$ & $%.3lf \\pm %.3lf$ \\\\\n", param.name.c_str(), tval, terr);
				else fprintf(f, "$%s$ & $%.3lf^{+%.3lf}_{-%.3lf}$ \\\\\n", param.name.c_str(), tval, terrp, terrm);
			}
		}
		fprintf(f, "$%s$ & $%.2lf/%d$ \\\\\n", "\\chi^2/N_{\\rm df}", m_Parameters.chi2ndf * GetNdf(), GetNdf());
		fprintf(f, "\\hline\n");
		fprintf(f, "\\end{tabular}\n");

		fclose(f);
}

std::string ThermalModelFit::GetCurrentTime() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[1000];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y at %I:%M:%S", timeinfo);
	std::string str(buffer);

	return str;
}

void ThermalModelFit::PrintFitLog(std::string filename, std::string comment, bool asymm)
{
	FILE *f;
	if (filename != "") {
		f = fopen(filename.c_str(), "w");
	}
	else {
		f = stdout;
	}
	if (comment != "")
		fprintf(f, "%s\n\n", comment.c_str());

	fprintf(f, "Performed on %s\n\n", GetCurrentTime().c_str());

	fprintf(f, "chi2/dof = %lf/%d = %lf\n\n", m_Parameters.chi2, m_Parameters.ndf, m_Parameters.chi2ndf);

	fprintf(f, "Extracted parameters:\n");
	for (int i = 0; i < 10 ; ++i) {
		if (i == 6 && m_model->Ensemble() != ThermalModelBase::SCE)
			continue;
		FitParameter param = m_Parameters.GetParameter(i);
		std::string sunit = "";
		std::string tname = "";
		double tmn = 1.;
		if ((i >= 0 && i <= 3) || i == 8) {
			sunit = "[MeV]";
			tmn   = 1.e3;
		}
		else if (i == 5 || i == 6) {
			sunit = "[fm]";
		}
		tname = param.name + sunit;

		if (param.name == "muS" && (m_model->Ensemble() == ThermalModelBase::SCE || m_model->Ensemble() == ThermalModelBase::CE))
			continue;

		if (param.name == "muB" && m_model->Ensemble() == ThermalModelBase::CE)
			continue;

		if (param.name == "muQ" && m_model->Ensemble() == ThermalModelBase::CE)
			continue;

		if ((param.name == "muS" || param.name == "gammaS") && !m_model->TPS()->hasStrange())
			continue;
		if (param.name == "muQ" && !m_model->TPS()->hasCharged())
			continue;
		if ((param.name == "muC" || param.name == "gammaC") && !m_model->TPS()->hasCharmed())
			continue;

		if (param.toFit) {
			double tval = param.value, terr = param.error, terrp = param.errp, terrm = param.errm;
			
			if (!asymm) 
				fprintf(f, "%15s = %15lf +- %-15lf\n", tname.c_str(), tval * tmn, terr * tmn);
			else 
				fprintf(f, "%15s = %15lf + %-15lf - %-15lf\n", tname.c_str(), tval * tmn, terrp * tmn, terrm * tmn);
		}
		else {
			double tval = param.value;
			fprintf(f, "%15s = %15lf (FIXED)\n", tname.c_str(), tval * tmn);
		}
	}

	fprintf(f, "\n\n");

	fprintf(f, "Yields:\n");
	for (int i = 0; i<m_Multiplicities.size(); ++i) {
		double dens1 = GetDensity(m_Multiplicities[i].fPDGID, m_Multiplicities[i].fFeedDown);

		std::string tname = m_model->TPS()->GetNameFromPDG(m_Multiplicities[i].fPDGID);
		if (m_Multiplicities[i].fPDGID == 1) tname = "Npart";
		else if (m_Multiplicities[i].fPDGID == 33340) tname = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
		fprintf(f, "%25s Experiment: %15lf +- %-15lf Model: %-15lf Std.dev.: %-15lf \n", tname.c_str(), m_Multiplicities[i].fValue, m_Multiplicities[i].fError, dens1 * m_model->Parameters().V, (dens1 * m_model->Parameters().V - m_Multiplicities[i].fValue) / m_Multiplicities[i].fError);
	}

	for (int i = 0; i<m_Ratios.size(); ++i) {
		double dens1 = GetDensity(m_Ratios[i].PDGID1, m_Ratios[i].fFeedDown1);
		double dens2 = GetDensity(m_Ratios[i].PDGID2, m_Ratios[i].fFeedDown2);

		std::string name1 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID1);
		std::string name2 = m_model->TPS()->GetNameFromPDG(m_Ratios[i].PDGID2);
		if (m_Ratios[i].PDGID1 == 1) name1 = "Npart";
		if (m_Ratios[i].PDGID2 == 1) name2 = "Npart";
		if (m_Ratios[i].PDGID1 == 33340) name1 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
		if (m_Ratios[i].PDGID2 == 33340) name2 = m_model->TPS()->Particles()[m_model->TPS()->PdgToId(3334)].Name() + " + " + m_model->TPS()->Particles()[m_model->TPS()->PdgToId(-3334)].Name();
		fprintf(f, "%25s Experiment: %15lf +- %-15lf Model: %-15lf Std.dev.: %-15lf \n", (std::string(name1 + "/" + name2)).c_str(), m_Ratios[i].fValue, m_Ratios[i].fError, dens1 / dens2, (dens1 / dens2 - m_Ratios[i].fValue) / m_Ratios[i].fError);
	}

	if (f != stdout)
		fclose(f);
}

using namespace std;

std::vector<FittedQuantity> ThermalModelFit::loadExpDataFromFile(const std::string & filename) {
	std::vector<FittedQuantity> ret(0);
	fstream fin;
	fin.open(filename.c_str());
	if (fin.is_open()) {
		char tmpc[2000];
		fin.getline(tmpc, 2000);
		string tmp = string(tmpc);
		vector<string> elems = CuteHRGHelper::split(tmp, '#');

		int flnew = 0;
		if (elems.size() < 1 || CuteHRGHelper::split(elems[0], ';').size() < 4)
			flnew = 1;
		else
			flnew = 0;

		fin.clear();
		fin.seekg(0, ios::beg);

		if (flnew == 1)
			ret = loadExpDataFromFile_NewFormat(fin);
		else
			ret = loadExpDataFromFile_OldFormat(fin);

		fin.close();
	}
	return ret;
}

std::vector<FittedQuantity> ThermalModelFit::loadExpDataFromFile_OldFormat(std::fstream & fin)
{
	std::vector<FittedQuantity> ret(0);
	if (fin.is_open()) {
		string tmp;
		char tmpc[2000];
		fin.getline(tmpc, 2000);
		tmp = string(tmpc);
		while (1) {
			vector<string> fields = CuteHRGHelper::split(tmp, ';');
			if (fields.size()<4) break;
			int type = atoi(fields[0].c_str());
			int pdgid1 = atoi(fields[1].c_str()), pdgid2 = 0;
			double value, error, error2;
			int feeddown1 = 1, feeddown2 = 1;
			if (type) {
				pdgid2 = atoi(fields[2].c_str());
				value = atof(fields[3].c_str());
				if (fields.size() >= 5) error = atof(fields[4].c_str());
				if (fields.size() >= 6) {
					error2 = atof(fields[5].c_str());
					error = sqrt(error*error + error2*error2);
				}
				if (fields.size() >= 7) feeddown1 = atoi(fields[6].c_str());
				if (fields.size() >= 8) feeddown2 = atoi(fields[7].c_str());
			}
			else {
				value = atof(fields[2].c_str());
				error = atof(fields[3].c_str());
				if (fields.size() >= 5) {
					error2 = atof(fields[4].c_str());
					error = sqrt(error*error + error2*error2);
				}
				if (fields.size() >= 6) feeddown1 = atoi(fields[5].c_str());
			}

			if (type) ret.push_back(FittedQuantity(ExperimentRatio(pdgid1, pdgid2, value, error, feeddown1, feeddown2)));
			else ret.push_back(FittedQuantity(ExperimentMultiplicity(pdgid1, value, error, feeddown1)));

			fin.getline(tmpc, 500);
			tmp = string(tmpc);
		}
		fin.close();
	}
	return ret;
}

std::vector<FittedQuantity> ThermalModelFit::loadExpDataFromFile_NewFormat(std::fstream & fin)
{
	std::vector<FittedQuantity> ret(0);
	if (fin.is_open()) {
		char cc[2000];
		while (!fin.eof()) {
			fin.getline(cc, 2000);
			string tmp = string(cc);
			vector<string> elems = CuteHRGHelper::split(tmp, '#');
			if (elems.size() < 1)
				continue;

			istringstream iss(elems[0]);

			int fitflag;
			int pdgid1, pdgid2;
			int feeddown1, feeddown2;
			double value, error;
			
			if (iss >> fitflag >> pdgid1 >> pdgid2 >> feeddown1 >> feeddown2 >> value >> error) {
				if (pdgid2 != 0) ret.push_back(FittedQuantity(ExperimentRatio(pdgid1, pdgid2, value, error, feeddown1, feeddown2)));
				else ret.push_back(FittedQuantity(ExperimentMultiplicity(pdgid1, value, error, feeddown1)));

				if (fitflag != 0)
					ret[ret.size() - 1].toFit = true;
				else
					ret[ret.size() - 1].toFit = false;
			}
		}
	}
	return ret;
}

void ThermalModelFit::saveExpDataToFile(const std::vector<FittedQuantity>& outQuantities, const std::string & filename)
{
	std::ofstream fout(filename.c_str());
	if (fout.is_open()) {
		fout << "#"
			<< std::setw(14) << "is_fitted"
			<< std::setw(15) << "pdg1"
			<< std::setw(15) << "pdg2"
			<< std::setw(15) << "feeddown1"
			<< std::setw(15) << "feeddown2"
			<< std::setw(15) << "value"
			<< std::setw(15) << "error"
			<< std::endl;
		for (int i = 0; i < outQuantities.size(); ++i) {
			if (outQuantities[i].type == FittedQuantity::Multiplicity) {
				fout << std::setw(15) << static_cast<int>(outQuantities[i].toFit)
					<< std::setw(15) << outQuantities[i].mult.fPDGID
					<< std::setw(15) << 0
					<< std::setw(15) << outQuantities[i].mult.fFeedDown
					<< std::setw(15) << 0
					<< std::setw(15) << outQuantities[i].mult.fValue
					<< std::setw(15) << outQuantities[i].mult.fError
					<< std::endl;
			}
			else {
				fout << std::setw(15) << static_cast<int>(outQuantities[i].toFit) 
					<< std::setw(15) << outQuantities[i].ratio.PDGID1
					<< std::setw(15) << outQuantities[i].ratio.PDGID2
					<< std::setw(15) << outQuantities[i].ratio.fFeedDown1
					<< std::setw(15) << outQuantities[i].ratio.fFeedDown2
					<< std::setw(15) << outQuantities[i].ratio.fValue
					<< std::setw(15) << outQuantities[i].ratio.fError
					<< std::endl;
			}
		}
		fout.close();
	}
	else {
		printf("**WARNING** ThermalModelFit::saveExpDataToFile: Cannot open file for writing data!");
	}
}

double ThermalModelFit::chi2Ndf(double T, double muB) {
#ifdef USE_MINUIT
	double Tinit = T, muBinit = muB;
	FitFCN mfunc(this);
	int nparams = 6;
	if (!m_Parameters.T.toFit) nparams--;
	if (!m_Parameters.muB.toFit) nparams--;
	if (!m_Parameters.gammaq.toFit) nparams--;
	if (!m_Parameters.gammaS.toFit) nparams--;
	if (!m_Parameters.R.toFit || (m_Multiplicities.size()==0 && m_model->Ensemble() == ThermalModelBase::GCE)) nparams--;
	if (!m_Parameters.Rc.toFit || m_model->Ensemble() != ThermalModelBase::SCE) nparams--;
	std::vector<double> params(6, 0.);
	params[0] = T;
	params[1] = muB;
	params[2] = m_model->Parameters().gammaS;
	params[3] = m_model->Parameters().V;
	params[4] = m_model->Parameters().SVc;
	params[5] = m_model->Parameters().gammaq;
	m_model->SetStrangenessChemicalPotential(muB / 5.);
	m_model->SetElectricChemicalPotential(-muB / 50.);
	double ret = mfunc(params);
	m_model->SetTemperature(Tinit);
	m_model->SetBaryonChemicalPotential(muBinit);
	return ret / (m_Multiplicities.size() + m_Ratios.size() - nparams);
#else
	return -1.;
#endif
}

int ThermalModelFit::GetNdf() const {
	int nparams = 8;
	if (!m_Parameters.T.toFit) nparams--;
	if (!m_Parameters.muB.toFit) nparams--;
	if (!m_Parameters.muS.toFit) nparams--;
	if (!m_Parameters.muQ.toFit) nparams--;
	if (!m_Parameters.gammaq.toFit) nparams--;
	if (!m_Parameters.gammaS.toFit) nparams--;
	if (!m_Parameters.R.toFit || (m_Multiplicities.size()==0 && m_model->Ensemble() == ThermalModelBase::GCE)) nparams--;
	if (!m_Parameters.Rc.toFit || m_model->Ensemble() != ThermalModelBase::SCE) nparams--;
	int ndof = 0;
	for (int i = 0; i < m_Quantities.size(); ++i)
		if (m_Quantities[i].toFit) ndof++;
	return (ndof - nparams);
}

double ThermalModelFit::GetDensity(int PDGID, int feeddown) const {
	return m_model->GetDensity(PDGID, feeddown);
}
