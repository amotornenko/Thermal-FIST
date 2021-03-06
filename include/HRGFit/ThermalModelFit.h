#ifndef THERMALMODELFIT_H
#define THERMALMODELFIT_H

#include <string>

#include "HRGBase/ThermalModelBase.h"
#include "HRGBase/xMath.h"

/**
*	Structure for an arbitrary fit parameter.
*/
struct FitParameter {
	double value;
	double error;
	double errp, errm;
	double xmin, xmax;
	bool toFit;
	std::string name;
	FitParameter(std::string name_="", bool toFit_=true, double val=1., double err=0., double vmin=-2., double vmax=2.):
		toFit(toFit_), value(val), error(err), errp(err), errm(-err), xmin(vmin), xmax(vmax), name(name_) {
	}
};

/**
*	Extended structure for calculating uncertainties in non-fit quantities resulting from
*	uncertanties in fit parameters.
*/
struct ThermalModelFitParametersExtended {
	FitParameter T, muB, muS, muQ, muC, gammaq, gammaS, gammaC, R, Rc;
	double chi2ndf;
	FitParameter nH, rhoB, rhoQ, en, entropy, pressure;//, eta;
	ThermalModelFitParametersExtended() { }
	ThermalModelFitParametersExtended(ThermalModelBase *model) {
		T.value = model->Parameters().T;
		muB.value = model->Parameters().muB;
		muS.value = model->Parameters().muS;
		muQ.value = model->Parameters().muQ;
		muC.value = model->Parameters().muC;
		gammaq.value = model->Parameters().gammaq;
		gammaS.value = model->Parameters().gammaS;
		gammaC.value = model->Parameters().gammaC;
		//R.value = model->Parameters.R;
		R.value  = 0.;
		Rc.value = 0.;
		nH.value = model->CalculateHadronDensity();
		rhoB.value = model->CalculateBaryonDensity();
		rhoQ.value = model->CalculateChargeDensity();
		en.value = model->CalculateEnergyDensity();
		entropy.value = model->CalculateEntropyDensity();
		pressure.value = model->CalculatePressure();
	}
};

/**
*	Structure holding information about parameters of a thermal fit.
*/
struct ThermalModelFitParameters {
	bool GCE;	// 0 - CE, 1 - GCE
	FitParameter T, muB, muS, muQ, muC, gammaq, gammaS, gammaC, R, Rc;
	double chi2, chi2ndf;
	int ndf;
	double fR;
	ThermalModelFitParameters(double T_=0.150, double muB_=0.200, double muS_=0.1, double muQ_=-0.01, double gammaS_=1., double V_ = 4000., double Rc_=1.)
	{
		T = FitParameter("T", true, T_, 0.05, 0.02, 0.500);
		muB = FitParameter("muB", true, muB_, 0.05, 0., 0.950);
		muS = FitParameter("muS", false, muS_, 0.05, -0.450, 0.450);
		muQ = FitParameter("muQ", false, muQ_, 0.05, -0.130, 0.130);
		muC = FitParameter("muC", false, 0.);
		gammaq = FitParameter("gammaq", false, 1.0, 0.5, 0.01, 3.);
		gammaS = FitParameter("gammaS", false, gammaS_, 0.5, 0.01,3.);
		gammaC = FitParameter("gammaC", false, 1.0, 0.5, 0.01, 10.);
		//V = FitParameter("V", true, V_, 2000., 1., 20000.);	// Volume no longer used
		R  = FitParameter("R", true, V_, 1.0, 0., 25.0);
		Rc = FitParameter("Rc", true, Rc_, 1.0, 0., 10.0);
		//fR = Rc_;
	}
	ThermalModelFitParameters(const ThermalModelParameters &params)//:																																																																														 //T(T_), muB(muB_), muS(muS_), muQ(muQ_), gammaS(gammaS_), R(R_)	
	{
		T = FitParameter("T", true, params.T, 0.05, 0.02, 0.500);
		muB = FitParameter("muB", true, params.muB, 0.05, 0., 0.950);
		muS = FitParameter("muS", false, params.muS, 0.05, -0.450, 0.450);
		muQ = FitParameter("muQ", false, params.muQ, 0.05, -0.130, 0.130);
		muC = FitParameter("muC", false, params.muC);
		gammaq = FitParameter("gammaq", false, params.gammaq, 0.5, 0.01, 3.);
		gammaS = FitParameter("gammaS", false, params.gammaS, 0.5, 0.01, 3.);
		gammaC = FitParameter("gammaC", false, params.gammaC, 0.5, 0.01, 10.);
		//V = FitParameter("V", true, V_, 2000., 1., 20000.);	// Volume no longer used
		R = FitParameter("R", true, pow(3. * params.V / 16. / xMath::Pi(), 1./3.), 1.0, 0., 25.0);
		Rc = FitParameter("Rc", true, pow(3. * params.SVc / 16. / xMath::Pi(), 1. / 3.), 1.0, 0., 10.0);
		//fR = Rc_;
	}
	FitParameter GetParameter(const std::string& name) const {
		if (T.name==name) return T;
		if (muB.name==name) return muB;
		if (muS.name==name) return muS;
		if (muQ.name==name) return muQ;
		if (muC.name==name) return muC;
		if (gammaq.name==name) return gammaq;
		if (gammaS.name==name) return gammaS;
		if (gammaC.name==name) return gammaC;
		//if (V.name==name) return V;
		if (R.name==name) return R;
		if (Rc.name==name) return Rc;
		return FitParameter();
	}
	FitParameter GetParameter(const int index) const {
		if (index==0) return T;
		if (index==1) return muB;
		if (index==2) return muS;
		if (index==3) return muQ;
		if (index==4) return gammaS;
		//if (index==5) return V;
		if (index==5) return R;
		if (index==6) return Rc;
		if (index==7) return gammaq;
		if (index==8) return muC;
		if (index==9) return gammaC;
		return FitParameter();
	}
	void SetParameter(const std::string& name, const FitParameter& param) {
		if (T.name==name) T = param;
		if (muB.name==name) muB = param;
		if (muS.name==name) muS = param;
		if (muQ.name==name) muQ = param;
		if (muC.name==name) muC = param;
		if (gammaq.name==name) gammaq = param;
		if (gammaS.name==name) gammaS = param;
		if (gammaC.name==name) gammaC = param;
		//if (V.name==name) V = param;
		if (R.name==name)  R  = param;
		if (Rc.name==name) Rc = param;
	}
	void SetParameter(const std::string& name, double val, double err, double xmin, double xmax) {
		if (T.name==name) T = FitParameter(name, T.toFit, val, err, xmin, xmax);
		if (muB.name==name) muB = FitParameter(name, muB.toFit, val, err, xmin, xmax);
		if (muS.name==name) muS = FitParameter(name, muS.toFit, val, err, xmin, xmax);
		if (muQ.name==name) muQ = FitParameter(name, muQ.toFit, val, err, xmin, xmax);
		if (muC.name==name) muC = FitParameter(name, muC.toFit, val, err, xmin, xmax);
		if (gammaq.name==name) gammaq = FitParameter(name, gammaq.toFit, val, err, xmin, xmax);
		if (gammaS.name==name) gammaS = FitParameter(name, gammaS.toFit, val, err, xmin, xmax);
		if (gammaC.name==name) gammaC = FitParameter(name, gammaC.toFit, val, err, xmin, xmax);
		//if (V.name==name) V = FitParameter(name, true, val, err, xmin, xmax);
		if (R.name==name) R = FitParameter(name, R.toFit, val, err, xmin, xmax);
		if (Rc.name==name) Rc = FitParameter(name, Rc.toFit, val, err, xmin, xmax);
	}
	void SetParameterValue(const std::string& name, double value) {
		if (T.name == name) T.value = value;
		if (muB.name == name) muB.value = value;
		if (muS.name == name) muS.value = value;
		if (muQ.name == name) muQ.value = value;
		if (muC.name == name) muC.value = value;
		if (gammaq.name == name) gammaq.value = value;
		if (gammaS.name == name) gammaS.value = value;
		if (gammaC.name == name) gammaC.value = value;
		//if (V.name==name) V.toFit = toFit;
		if (R.name == name) R.value = value;
		if (Rc.name == name) Rc.value = value;
	}
	void SetParameterFitFlag(const std::string& name, bool toFit) {
		if (T.name==name) T.toFit = toFit;
		if (muB.name==name) muB.toFit = toFit;
		if (muS.name==name) muS.toFit = toFit;
		if (muQ.name==name) muQ.toFit = toFit;
		if (muC.name==name) muC.toFit = toFit;
		if (gammaq.name==name) gammaq.toFit = toFit;
		if (gammaS.name==name) gammaS.toFit = toFit;
		if (gammaC.name==name) gammaC.toFit = toFit;
		//if (V.name==name) V.toFit = toFit;
		if (R.name==name) R.toFit = toFit;
		if (Rc.name==name) Rc.toFit = toFit;
	}
	ThermalModelParameters GetThermalModelParameters() {
		ThermalModelParameters ret(T.value, muB.value, muS.value, muQ.value, gammaS.value, 4. / 3. * xMath::Pi() * R.value * R.value * R.value);
		ret.SVc = 4. / 3. * xMath::Pi() * Rc.value * Rc.value * Rc.value;
		ret.gammaq = gammaq.value;
		ret.muC    = muC.value;
		ret.gammaC = gammaC.value;
		return ret;
	}
};

struct ExperimentMultiplicity {
	int fPDGID;
	double fValue, fError;
	int fFeedDown; /// 0 - primordial, 1 - +decays from unstable, 2 - +weak decays
	ExperimentMultiplicity(int PDGID=-211, double value=300., double error=20., int fd = 1):
		fPDGID(PDGID), fValue(value), fError(error), fFeedDown(fd) { }
	void addSystematicError(double percent) {
		fError = sqrt(fError*fError + percent*percent*fValue*fValue);
	}
};

struct ExperimentRatio {
	int PDGID1, PDGID2;
	double fValue, fError;
	int fFeedDown1, fFeedDown2; /// 0 - primordial, 1 - +decays from unstable, 2 - +weak decays
	ExperimentRatio(int PDGID1_ = 211, int PDGID2_ = -211, double value_ = 1., double error_ = 0.1, int fd1 = 1, int fd2 = 1):
		PDGID1(PDGID1_), PDGID2(PDGID2_), fValue(value_), fError(error_), fFeedDown1(fd1), fFeedDown2(fd2) { }
	ExperimentRatio(int PDGID1_, int PDGID2_, double value1, double error1, double value2, double error2, int fd1 = 1, int fd2 = 1):
		PDGID1(PDGID1_), PDGID2(PDGID2_), fFeedDown1(fd1), fFeedDown2(fd2) { 
			fValue = value1 / value2;
			fError = sqrt(error1*error1 / value2 / value2 + value1 * value1 / value2  / value2  / value2  / value2  * error2 * error2);
	}
};

struct FittedQuantity {
	enum FittedQuantityType { Multiplicity = 0, Ratio = 1};
	FittedQuantityType type; // 0 - Multiplicity, 1 - Ratio
	bool toFit;
	ExperimentMultiplicity mult;
	ExperimentRatio ratio;
	FittedQuantity() {
		toFit = true;
		type = FittedQuantity::Multiplicity;
		mult = ExperimentMultiplicity(-211, 10., 1.);
	}
	FittedQuantity(const ExperimentMultiplicity & op) {
		toFit = true;
		type = FittedQuantity::Multiplicity;
		mult = op;
	}
	FittedQuantity(const ExperimentRatio & op) {
		toFit = true;
		type = FittedQuantity::Ratio;
		ratio = op;
	}
};

class ThermalModelBase;

class ThermalModelFit
{
	public:
		ThermalModelFit(ThermalModelBase *model_);

		~ThermalModelFit(void);

		double GetDensity(int PDGID, int feeddown) const;

		void SetFitFlag(const std::string& name, bool flag) {
			m_Parameters.SetParameterFitFlag(name, flag);
			//m_QBgoal = 0.4;
		}

		void SetQBConstraint(double QB) {
			m_QBgoal = QB;
			if (m_model != NULL)
				m_model->SetQoverB(QB);
		}

		void SetQuantities(const std::vector<FittedQuantity> & inQuantities) {
			m_Quantities    .resize(0);
			m_Ratios        .resize(0);
			m_Multiplicities.resize(0);
			AddQuantities(inQuantities);
		}

		void AddQuantities(const std::vector<FittedQuantity> & inQuantities) {
			for (int i = 0; i < inQuantities.size(); ++i)
				AddQuantity(inQuantities[i]);
		}

		void AddQuantity(const FittedQuantity& inQuantity) {
			m_Quantities.push_back(inQuantity);
			if (inQuantity.type == FittedQuantity::Ratio)
				m_Ratios.push_back(inQuantity.ratio);
			else
				m_Multiplicities.push_back(inQuantity.mult);
		}

		// To be deprecated
		void SetRatios(const std::vector<ExperimentRatio> & inRatios) {
			m_Ratios = inRatios;
		}

		void AddRatios(const std::vector<ExperimentRatio> & inRatios) {
			m_Ratios.insert(m_Ratios.end(), inRatios.begin(), inRatios.end());
			for (int i = 0; i < inRatios.size(); i++) {
				m_Quantities.push_back(FittedQuantity(inRatios[i]));
			}
		}

		void AddRatio(const ExperimentRatio& inRatio) {
			m_Ratios.push_back(inRatio);
			m_Quantities.push_back(FittedQuantity(inRatio));
		}

		void ClearRatios() { m_Ratios.resize(0); }

		void PrintRatios();

		// To be deprecated
		void SetMultiplicities(const std::vector<ExperimentMultiplicity> & inMultiplicities) {
			m_Multiplicities = inMultiplicities;
		}

		void AddMultiplicities(const std::vector<ExperimentMultiplicity> & inMultiplicities) {
			m_Multiplicities.insert(m_Multiplicities.end(), inMultiplicities.begin(), inMultiplicities.end());
			for (int i = 0; i < inMultiplicities.size(); i++) {
				m_Quantities.push_back(FittedQuantity(inMultiplicities[i]));
			}
		}

		void AddMultiplicity(const ExperimentMultiplicity& inMultiplicity) {
			m_Multiplicities.push_back(inMultiplicity);
			m_Quantities.push_back(FittedQuantity(inMultiplicity));
		}

		void ClearMultiplicities() { m_Multiplicities.resize(0); }

		void PrintParameters();

		void PrintMultiplicities();

		void PrintYields();

		void PrintYieldsTable(std::string filename="Yield.dat");

		void PrintYieldsTable2(std::string filename="Yield.dat");

		void PrintYieldsLatex(std::string filename="Yield.dat", std::string name="A+A");

		void PrintYieldsLatexAll(std::string filename="Yield.dat", std::string name="A+A", bool asymm = false);

		static std::string GetCurrentTime();
		void PrintFitLog(std::string filename = "", std::string comment = "Thermal fit", bool asymm = false);

		double chi2Ndf(double T, double muB);

		ThermalModelFitParameters PerformFit(bool verbose = true, bool AsymmErrors = false);

		int GetNdf() const;

		void Increment() { m_Iters++; }

		const ThermalModelFitParameters& Parameters() const { return m_Parameters; }
		void SetParameters(const ThermalModelFitParameters& params)  { m_Parameters = params; }
		void SetParameter(const std::string & name, const FitParameter & param) { m_Parameters.SetParameter(name, param); }
		void SetParameter(const std::string & name, double val, double err, double xmin, double xmax) { m_Parameters.SetParameter(name, val, err, xmin, xmax); }
		void SetParameterValue(const std::string & name, double value) { m_Parameters.SetParameterValue(name, value); }
		void SetParameterFitFlag(const std::string & name, bool toFit) { m_Parameters.SetParameterFitFlag(name, toFit); }
		const ThermalModelFitParametersExtended& ExtendedParameters() const { return m_ExtendedParameters; }
		ThermalModelBase* model() { return m_model; }

		double QoverB() const { return m_QBgoal; }

		const std::vector<ExperimentMultiplicity>&  	Multiplicities()	const { return m_Multiplicities; }
		const std::vector<ExperimentRatio>&					Ratios()					const { return m_Ratios; }
		const std::vector<FittedQuantity>&					FittedQuantities()					const { return m_Quantities; }

		int& Iters()		{ return m_Iters; }
		double& Chi2()	{ return m_Chi2;	}
		double& BT()		{ return m_BT;		}
		double& QT()		{ return m_QT;		}
		double& ST()		{ return m_ST;		}
		double& CT()    { return m_CT; }
		int& Ndf()			{ return m_Ndf;		}


    static std::vector<FittedQuantity> loadExpDataFromFile(const std::string & filename);
		static std::vector<FittedQuantity> loadExpDataFromFile_OldFormat(std::fstream & fin);
		static std::vector<FittedQuantity> loadExpDataFromFile_NewFormat(std::fstream & fin);
		static void saveExpDataToFile(const std::vector<FittedQuantity> & outQuantities, const std::string & filename);

	private:
		ThermalModelFitParameters m_Parameters;
		ThermalModelFitParametersExtended m_ExtendedParameters;
		ThermalModelBase *m_model;
		double m_QBgoal;
		std::vector<ExperimentMultiplicity> m_Multiplicities;
		std::vector<ExperimentRatio> m_Ratios;
		std::vector<FittedQuantity> m_Quantities;
		int			m_Iters;
		double		m_Chi2;
		double		m_BT;
		double		m_QT;
		double		m_ST;
		double		m_CT;
		int			m_Ndf;
};



#endif
