#include "TChain.h"
#include "TFile.h"

#include "VK.h"
#include "RTRelationPoly.h"

#include "v_plot.hxx"

using namespace std;
using namespace CS;

////////////////////////////////////////////////////////////////////////////////

int NameToNumber(const char *det){
	if     (strcmp(det, "ST03X1ub") == 0)
		return 0;
	else if(strcmp(det, "ST03X1db") == 0)
		return 1;
	else if(strcmp(det, "ST03Y1ub") == 0)
		return 2;
	else if(strcmp(det, "ST03Y1db") == 0)
		return 3;
	else if(strcmp(det, "ST03U1ub") == 0)
		return 4;
	else if(strcmp(det, "ST03U1db") == 0)
		return 5;
	else if(strcmp(det, "ST03V1ub") == 0)
		return 6;
	else if(strcmp(det, "ST03V1db") == 0)
		return 7;
	else if(strcmp(det, "ST03Y2ub") == 0)
	 	return 8;
	else if(strcmp(det, "ST03Y2db") == 0)
		return 9;
	else if(strcmp(det, "ST03X2ub") == 0)
		return 10;
	else if(strcmp(det, "ST03X2db") == 0)
		return 11;
	else 
		return 12;	
}

////////////////////////////////////////////////////////////////////////////////

void VK::VCreate(const char *src,const char *det, int ch_start, int ch_end, int pos, int delta,const char *cuts,vector<VData> &vdata)
{
	TChain *tree = new TChain("myTuple_");
	char tuple[200];
	for(int i=0; i<6; i++){
		if(i==0)
			sprintf(tuple, "%salign.root", src);
		else
			sprintf(tuple, "%salign%d.root", src, i);
	tree->Add(tuple);
	}
	
	// find correct entries from detector names
	int layer_nr = NameToNumber(det);
	char channel[10];
	char x_koord[10];
	char y_koord[10];
	char detect_id[10];
	char time[10];
	
	sprintf(channel, "ch%d", layer_nr);
	sprintf(time, "time%d", layer_nr);
	sprintf(detect_id, "pID%d", layer_nr);
	// treat bug in ntuple creation well
	if(layer_nr==2 || layer_nr==3 ||  layer_nr==8 || layer_nr==9){
        	sprintf(x_koord, "y%d", layer_nr);
		sprintf(y_koord, "x%d", layer_nr);
		}
	else	{
		sprintf(x_koord, "x%d", layer_nr);
		sprintf(y_koord, "y%d", layer_nr);
		}
	///////////
	
	float ch,d,wy,t,dc2H,chi2,gemH,pid;
	tree->SetBranchAddress(channel,&ch);
	tree->SetBranchAddress(y_koord,&wy);
	tree->SetBranchAddress(x_koord,&d);
	tree->SetBranchAddress(time,&t);
	tree->SetBranchAddress(detect_id,&pid);
	tree->SetBranchAddress("chiQC",&chi2);
	tree->SetBranchAddress("planesDC2",&dc2H);
	tree->SetBranchAddress("gemHit",&gemH);

	for(int i=0; i<tree->GetEntries(); i++){
		tree->GetEvent(i);
		
		if(int(ch)<ch_start || int(ch)>ch_end)
			continue;
		if (fabs(wy-pos)>delta)
			continue;
		if (chi2>200. || (dc2H<=4 && gemH<=6))
			continue;
		if ((int(fmod(double(pid), 10.)))== 1){
			vdata.push_back( VData() );
			vdata.back().x = d;
			vdata.back().t = t;
			vdata.back().w = 1;
			}
	}
        delete tree;
}

////////////////////////////////////////////////////////////////////////////////

void VK::VFit(VFitResult &result)
{
    RTRelationPoly *ppp=new RTRelationPoly();
    ppp->SetRMax(2.7);
    ppp->GetPoly().push_back(9.18292);
    ppp->GetPoly().push_back(2.5395);
    ppp->GetPoly().push_back(-0.807645);
    ppp->GetPoly().push_back(-0.282376);
    ppp->GetPoly().push_back(0.0341231);
    ppp->GetPoly().push_back(0.0528839);
    ppp->GetPoly().push_back(-0.0088662);
    result.rt = ppp;

    int n=0;
    for( unsigned int i=0; i<result.vdata.size(); i++ )
        n+= int(result.vdata[i].w);

    // to be filled
    V_plot vplot(n+1);
    int counter = 0;
    for( unsigned int i=0; i<result.vdata.size(); i++ )
    {  
        const VData &v=result.vdata[i];
	for( int j=0; j<int(v.w); j++ )
	{
	  counter ++;
	      	vplot.set_t(counter, v.t);
		vplot.set_r(counter, v.x);
		vplot.set_error(counter, 0.1);
	}
    }

/////   remove background and fit  ///////////////
 vplot.remove_background_V(5, 2.7);
 vplot.fit_fixed_params_V(9);
 result.w0=vplot.r_0();
 result.t0=vplot.t_0();
 result.rt->SetT0(result.t0);
 
    result.success=true;
}

////////////////////////////////////////////////////////////////////////////////

void VK::VStore(void *location, const std::string &name, const VFitResult &result)
{
    ofstream output;
    output.open(result.detector.c_str(), ofstream::out | ofstream::app);
    output.precision(9);
    output << result.w0 << "     " << 0.01 << "     ";
    output << result.t0 << "     " << 0.2  << endl;
    output.close();

    V::VStore(location,name,result);
}

////////////////////////////////////////////////////////////////////////////////

extern "C" {

V *VConstruct(void)
{
    return new VK;
}

void VDestroy(V *v)
{
    delete v;
}

}

////////////////////////////////////////////////////////////////////////////////
