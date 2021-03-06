#ifndef THERMALMODELIDEAL_H
#define THERMALMODELIDEAL_H

#include "HRGBase/ThermalModelBase.h"


class ThermalModelIdeal : public ThermalModelBase
{
	public:
		ThermalModelIdeal(ThermalParticleSystem *TPS_, const ThermalModelParameters& params = ThermalModelParameters());

		virtual ~ThermalModelIdeal(void);

		virtual void CalculateDensities();

		virtual void CalculateTwoParticleCorrelations();

		virtual void CalculateFluctuations();

		virtual double CalculateEnergyDensity();

		virtual double CalculateEntropyDensity();

		virtual double CalculateBaryonMatterEntropyDensity();

		virtual double CalculateMesonMatterEntropyDensity();

		virtual double CalculatePressure();

		virtual double CalculateShearViscosity();

		virtual double CalculateHadronScaledVariance();

		virtual double CalculateParticleScaledVariance(int part);

		virtual double CalculateParticleSkewness(int part);

		virtual double CalculateParticleKurtosis(int part);

		virtual double ParticleScalarDensity(int part);
};

#endif
