#ifndef THERMALMODELBASE_H
#define THERMALMODELBASE_H

#include <string>

#include "HRGBase/ThermalParticleSystem.h"
#include "HRGBase/xMath.h"




class ThermalModelBase
{
	public:
		enum ThermalModelEnsemble { GCE = 0, CE = 1, SCE = 2, CCE = 3 };
		enum ThermalModelInteraction { Ideal = 0, DiagonalEV = 1, CrosstermsEV = 2, QvdW = 3, RealGas = 4, MeanField = 5 };

		ThermalModelBase(ThermalParticleSystem *TPS_, const ThermalModelParameters& params = ThermalModelParameters());

		virtual ~ThermalModelBase(void) { }
		virtual void FillVirial(const std::vector<double> & ri = std::vector<double>(0)) { }

		bool UseWidth() const { return m_UseWidth; }
		void SetUseWidth(bool useWidth) { m_UseWidth = useWidth; }
		
		bool NormBratio() const { return m_NormBratio; }
		void SetNormBratio(bool normBratio);
		void SetOMP(bool openMP) { m_useOpenMP = openMP; }
		//void SetHagedorn(bool useHagedorn, double M0 = 3., double TH = 0.160, double a = 3., double ` = 1.);
		void ResetChemicalPotentials();

		//virtual void SetParameters(double T, double muB, double muS, double muQ, double gammaS, double V, double R);
		const ThermalModelParameters& Parameters() const { return m_Parameters; }
		virtual void SetParameters(const ThermalModelParameters& params);

		virtual void SetTemperature										(double T	 );
		virtual void SetBaryonChemicalPotential				(double muB);
		virtual void SetElectricChemicalPotential			(double muQ);
		virtual void SetStrangenessChemicalPotential		(double muS);
		virtual void SetCharmChemicalPotential					(double muC);
		virtual void SetGammaq													(double gammaq);
		virtual void SetGammaS													(double gammaS);
		virtual void SetGammaC													(double gammaC);
		virtual void SetBaryonCharge										(int B);
		virtual void SetElectricCharge									(int Q);
		virtual void SetStrangeness										(int S);
		virtual void SetCharm													(int C);

		virtual void SetRadius(double rad) { }
		virtual void SetVirial(int i, int j, double b) { }
		virtual void SetAttraction(int i, int j, double b) { }

		virtual void ReadInteractionParameters(const std::string &filename) { }
		virtual void WriteInteractionParameters(const std::string &filename) { }

		virtual void ChangeTPS(ThermalParticleSystem *TPS_);

		virtual double VirialCoefficient(int i, int j) const { return 0.; }
		virtual double AttractionCoefficient(int i, int j) const { return 0.; }

		// 0 - Boltzmann, 1 - Quantum
		bool QuantumStatistics() const { return m_QuantumStats; }
		virtual void SetStatistics(bool stats);

		virtual void SetCalculationType(IdealGasFunctions::QStatsCalculationType type) { m_TPS->SetCalculationType(type); }
		virtual void SetClusterExpansionOrder(int order) { m_TPS->SetClusterExpansionOrder(order); }
		void SetResonanceWidthShape(ThermalParticle::ResonanceWidthShape shape) { m_TPS->SetResonanceWidthShape(shape); }
		void SetResonanceWidthIntegrationType(ThermalParticle::ResonanceWidthIntegration type) { m_TPS->SetResonanceWidthIntegrationType(type); }

		virtual void FillChemicalPotentials();
		virtual void SetChemicalPotentials(const std::vector<double> & chem = std::vector<double>(0));

		//virtual void SetMesonsPoint();

		bool ConstrainMuB() const { return m_ConstrainMuB; }
		void ConstrainMuB(bool constrain) { m_ConstrainMuB = constrain; }
		bool ConstrainMuQ() const { return m_ConstrainMuQ; }
		void ConstrainMuQ(bool constrain) { m_ConstrainMuQ = constrain; }
		bool ConstrainMuS() const { return m_ConstrainMuS; }
		void ConstrainMuS(bool constrain) { m_ConstrainMuS = constrain; }
		bool ConstrainMuC() const { return m_ConstrainMuC; }
		void ConstrainMuC(bool constrain) { m_ConstrainMuC = constrain; }

		double QoverB() const { return m_QBgoal; }
		void SetQoverB(double QB) { m_QBgoal = QB; }

		double Volume() const { return m_Parameters.V; }
		void SetVolume(double Volume) { m_Volume = Volume; m_Parameters.V = Volume; }
		void SetVolumeRadius(double Radius) { m_Volume = 4. / 3.*xMath::Pi() * Radius * Radius * Radius; m_Parameters.V = m_Volume; }

		double StrangenessCanonicalVolume() const { return CanonicalVolume(); }
		void SetStrangenessCanonicalVolume(double Volume) { SetCanonicalVolume(Volume); }
		void SetStrangenessCanonicalVolumeRadius(double Radius) { SetCanonicalVolumeRadius(Radius); }
		
		double CanonicalVolume() const { return m_Parameters.SVc; }
		void SetCanonicalVolume(double Volume) { m_Parameters.SVc = Volume; }
		void SetCanonicalVolumeRadius(double Radius) { m_Parameters.SVc = 4. / 3. * xMath::Pi() * Radius * Radius * Radius; }

		virtual void FixParameters();
		virtual void FixParametersNoReset();
		virtual void FixParameters(double QB);		// And zero net strangeness


		virtual void SolveChemicalPotentials(double totB = 0., double totQ = 0., double totS = 0., double totC = 0.,
																					double muBinit = 0., double muQinit = 0., double muSinit = 0., double muCinit = 0.);


		virtual void CalculateDensities() = 0;
		virtual void CalculateDensitiesGCE() { CalculateDensities(); m_GCECalculated = true; }

		virtual void CalculateFeeddown();

		virtual void CalculateTwoParticleCorrelations();
		
		virtual void CalculateFluctuations();

		virtual double CalculateHadronDensity();
		virtual double GetParticlePrimordialDensity(unsigned int);
		virtual double GetParticleTotalDensity(unsigned int);
		virtual double CalculateBaryonDensity();
		virtual double CalculateChargeDensity();
		virtual double CalculateStrangenessDensity();
		virtual double CalculateCharmDensity();
		virtual double CalculateAbsoluteBaryonDensity();
		virtual double CalculateAbsoluteChargeDensity();
		virtual double CalculateAbsoluteStrangenessDensity();
		virtual double CalculateAbsoluteCharmDensity();
		virtual double CalculateArbitraryChargeDensity();
		virtual double CalculateEnergyDensity() = 0;
		virtual double CalculateEntropyDensity() = 0;
		virtual double CalculateBaryonMatterEntropyDensity() { return 0.; }
		virtual double CalculateMesonMatterEntropyDensity() { return 0.; }
		virtual double CalculatePressure() = 0;
		virtual double CalculateShearViscosity() { return 0.; }
		virtual double CalculateHadronScaledVariance() { return 1.; }
		virtual double CalculateParticleScaledVariance(int) { return 1.; }
		virtual double CalculateParticleSkewness(int) { return 1.; }
		virtual double CalculateParticleKurtosis(int) { return 1.; }
		virtual double CalculateBaryonScaledVariance(bool susc = false) { return 1.; }
		virtual double CalculateChargeScaledVariance(bool susc = false) { return 1.; }
		virtual double CalculateStrangenessScaledVariance(bool susc = false) { return 1.; }
		virtual double CalculateEigenvolumeFraction() { return 0.; }
		virtual double ParticleScalarDensity(int part) = 0;

		virtual double GetMaxDiff() const { return m_MaxDiff; }
		virtual bool   IsLastSolutionOK() const { return m_LastCalculationSuccessFlag; }
		double GetDensity(int PDGID, int feeddown);

		std::vector<double> GetIdealGasDensities() const;

		ThermalParticleSystem* TPS() { return m_TPS; }

		const std::vector<double>& Densities()				const { return m_densities; }
		const std::vector<double>& TotalDensities() const { return m_densitiestotal; }

		const std::string& TAG() const { return m_TAG; }
		void setTAG(const std::string & tag) { m_TAG = tag; }

		bool IsCalculated() const { return m_Calculated; }
		bool IsFluctuationsCalculated() const { return m_FluctuationsCalculated; }
		bool IsGCECalculated() const { return m_GCECalculated; }

		double ScaledVariancePrimordial(int id) const { return (id >= 0 && id < m_wprim.size())    ? m_wprim[id]    : 1.; }
		double ScaledVarianceTotal     (int id) const { return (id >= 0 && id < m_wtot.size())     ? m_wtot[id]     : 1.; }
		double SkewnessPrimordial      (int id) const { return (id >= 0 && id < m_skewprim.size()) ? m_skewprim[id] : 1.; }
		double SkewnessTotal           (int id) const { return (id >= 0 && id < m_skewtot.size())  ? m_skewtot[id]  : 1.; }
		double KurtosisPrimordial      (int id) const { return (id >= 0 && id < m_kurtprim.size()) ? m_kurtprim[id] : 1.; }
		double KurtosisTotal           (int id) const { return (id >= 0 && id < m_kurttot.size())  ? m_kurttot[id]  : 1.; }

		double Susc(int i, int j) const { return m_Susc[i][j]; }

		double ChargedMultiplicity  (int type = 0); /// 0 - charged, 1 - +Q, -1 - -Q
		double ChargedScaledVariance(int type = 0); /// 0 - charged, 1 - +Q, -1 - -Q
		double ChargedMultiplicityFinal(int type = 0); /// 0 - charged, 1 - +Q, -1 - -Q
		double ChargedScaledVarianceFinal(int type = 0); /// 0 - charged, 1 - +Q, -1 - -Q

		ThermalModelEnsemble Ensemble() { return m_Ensemble; }
		ThermalModelInteraction InteractionModel() { return m_InteractionModel;  }

	protected:
		ThermalModelParameters m_Parameters;
		ThermalParticleSystem* m_TPS;

		bool   m_LastCalculationSuccessFlag;
		double m_MaxDiff;

		bool m_Calculated;
		bool m_FluctuationsCalculated;
		bool m_GCECalculated;
		bool m_UseWidth;
		bool m_NormBratio;
		bool m_QuantumStats;
		double m_QBgoal;
		double m_Volume;

		bool m_ConstrainMuB;
		bool m_ConstrainMuQ;
		bool m_ConstrainMuS;
		bool m_ConstrainMuC;

		bool m_useOpenMP;

		std::vector<double> m_densities;
		std::vector<double> m_densitiestotal;
		std::vector<double> m_densitiestotalweak;
		std::vector<double> m_Chem;

		// Scaled variance
		std::vector<double> m_wprim;
		std::vector<double> m_wtot;

		// Skewness
		std::vector<double> m_skewprim;
		std::vector<double> m_skewtot;

		// Kurtosis
		std::vector<double> m_kurtprim;
		std::vector<double> m_kurttot;

		// 2nd order correlations of primordial and total numbers
		std::vector< std::vector<double> > m_PrimCorrel;
		std::vector< std::vector<double> > m_TotalCorrel;

		// Conserved charges susceptibility matrix
		std::vector< std::vector<double> > m_Susc;

		double m_wnSum;

		std::string m_TAG;

		ThermalModelEnsemble m_Ensemble;
		ThermalModelInteraction m_InteractionModel;

		virtual void CalculateTwoParticleFluctuationsDecays();
		virtual void CalculateSusceptibilityMatrix();
};

#endif // THERMALMODELBASE_H
