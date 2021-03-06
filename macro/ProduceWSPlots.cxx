//===========================
// File: ProduceWSPlots.cxx
// Descriptions:
//  You can generate timing PDF used in FitVertexLE.
//  In the version 0, this requires the histograms of the t-TOF distribution which generated by nother script.
// Usage:
//  $ root 'ProduceWSPlots.cxx("{file name containing the histograms}", "{file name of output with generated PDF}")' 
// Version: 1.0
//===========================

#include <TFile.h>
#include <TLegend.h>
#include <TMath.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TSpline.h>
#include <iostream>

using namespace std;

void ProduceWSPlots(string fin = "hist.dump.root", string fout = "timePDF.root"){
    const int nFiles=1;
    TFile * _f[nFiles];
    const int nPMTtypes = 2;
    double DRTotalPerNS[nPMTtypes];

    bool isNew=true;//false;
    bool isNew2=false;

    int colorScale[nPMTtypes];
    for(int i=0;i<nPMTtypes;i++){
        if(i==0) colorScale[i] = 4;
        else if(i==1) colorScale[i] = 2;
        else colorScale[i] = 3;
    }

    int colorScale2[4];
    for(int i=0;i<4;i++){
        if(i==0) colorScale2[i] = 2;
        else if(i==1) colorScale2[i] = 4;
        else if(i==2) colorScale2[i] = 416+2;
        else colorScale2[i] = 1;
    }

    TLegend * l = new TLegend(0.65,0.65,0.89,0.89);
    l->SetLineColor(0);

    TFile * fOut = new TFile(fout.c_str(),"recreate");

    TH1D * ChargeProfile[nFiles][nPMTtypes];TCanvas * cChargeProfile;TH1D * ratioChargeProfile[nFiles][nPMTtypes];
    TH1D * ChargePerPMT[nFiles][nPMTtypes];TCanvas *cChargePerPMT;
    TH1D * TotalCharge[nFiles][nPMTtypes];TCanvas *cTotalCharge; TH1D * TotalHit[nFiles][nPMTtypes]; TCanvas *cTotalHit;

    TH1D * TimeProfile[nFiles][nPMTtypes];TCanvas * cTimeProfile;
    TH1D * TimeTOFProfile[nFiles][nPMTtypes];TCanvas * cTimeTOFProfile;
    TH1D * HitTimeTOFProfile[nFiles][nPMTtypes];TCanvas * cHitTimeTOFProfile;
    TH1D * HitTimeTOFDR[nFiles][nPMTtypes];

    TH2D * TimeTOFProfileXTOF[nFiles][nPMTtypes];TCanvas * cTimeTOFProfileXTOF;

    TH3D * angularResponseXPMT_hits[nFiles][nPMTtypes];TCanvas * cAngularResponsePMT_hits; TCanvas *cAngularResponsePMT_hits_1D;
    TH3D * angularResponseXPMT_allPMTs[nFiles][nPMTtypes];TCanvas * cAngularResponsePMT_allPMTs;
    //TH3D * ChargeProfile2DXPMT[nFiles][nPMTtypes];TCanvas * cChargeProfile2DXPMT;

    TH2D * ChargeProfileXdWall[nFiles][nPMTtypes];TCanvas * cChargeProfileXdWall;//TH1D * ratioChargeProfile[nFiles][nPMTtypes];

    const int nBinsTOF = 10;
    TH1D * TimeTOFProfileXTOF_1D[nFiles][nPMTtypes][nBinsTOF];TCanvas * cTimeTOFProfileXTOF_1D;

    TF1 * gausExpoConv[nFiles][nPMTtypes];
    TF1 * gausExpoConv2[nFiles][nPMTtypes];
    TF1 * ExpoQueue[nFiles][nPMTtypes];
    TF1 * gausExpoConv_slice[nFiles][nPMTtypes][nBinsTOF];
    TSpline3 * splineExpoConv[nFiles][nPMTtypes];
    TSpline3 * splineExpoConv2[nFiles][nPMTtypes];
    TSpline3 * splineExpoQueue[nFiles][nPMTtypes];
    TSpline3 * splineDR[nFiles][nPMTtypes];
    double limitFitGausExpo[nPMTtypes];
    for(int i=0;i<nPMTtypes;i++){
        if(i==0) limitFitGausExpo[i] = 800;//100
        else limitFitGausExpo[i]=800;//100
    }
    TGraph * graphExpoQueue[nFiles][nPMTtypes];
    TGraph * graphDR[nFiles][nPMTtypes];
    TH3D * hPMTDirectionality[nPMTtypes];


    for(int f=0;f<nFiles;f++){

        _f[f] = new TFile(fin.c_str(), "read");


        double nEvents=0;
        double events[nPMTtypes];

        for(int i=0;i<nPMTtypes;i++){

            ChargePerPMT[f][i] = (TH1D*) _f[f]->Get(Form("ChargePerPMT_pmtType%d",i));
            ChargePerPMT[f][i]->SetLineWidth(2);
            ChargePerPMT[f][i]->SetLineColor(colorScale[i]);
            ChargePerPMT[f][i]->GetXaxis()->SetTitle("Q (p.e)");
            ChargePerPMT[f][i]->GetYaxis()->SetTitle("Number of events (normalized)");
            if(f==0){
                if(i==0) l->AddEntry(ChargePerPMT[f][i],"B&L");
                else if(i==1) l->AddEntry(ChargePerPMT[f][i],"multi-PMT");
            }

            ChargeProfile[f][i] = (TH1D*) _f[f]->Get(Form("ChargeProfile_pmtType%d",i));
            ChargeProfile[f][i]->SetLineWidth(2);
            ChargeProfile[f][i]->SetLineColor(colorScale[i]);
            ChargeProfile[f][i]->GetXaxis()->SetTitle("Angle with particle direction (#circ)");
            ChargeProfile[f][i]->GetYaxis()->SetTitle("Charge (p.e)");

            if(isNew2){
                ChargeProfileXdWall[f][i] = (TH2D*) _f[f]->Get(Form("ChargeProfileXdWall_pmtType%d",i));
                ChargeProfileXdWall[f][i]->GetXaxis()->SetTitle("dwall (cm)");
                ChargeProfileXdWall[f][i]->GetYaxis()->SetTitle("Angle with particle direction (#circ)");
            }

            TotalCharge[f][i] = (TH1D*) _f[f]->Get(Form("TotalCharge_pmtType%d",i));
            TotalCharge[f][i]->SetLineWidth(2);
            TotalCharge[f][i]->SetLineColor(colorScale[i]);
            TotalCharge[f][i]->GetXaxis()->SetTitle("Q (p.e)");
            TotalCharge[f][i]->GetYaxis()->SetTitle("Number of events (normalized)");
            nEvents += TotalCharge[f][i]->Integral();

            events[i] = TotalCharge[f][i]->Integral();

            TotalHit[f][i] = (TH1D*) _f[f]->Get(Form("TotalHit_pmtType%d",i));
            TotalHit[f][i]->SetLineWidth(2);
            TotalHit[f][i]->SetLineColor(colorScale[i]);
            TotalHit[f][i]->GetXaxis()->SetTitle("Q (p.e)");
            TotalHit[f][i]->GetYaxis()->SetTitle("Number of events (normalized)");

            TimeProfile[f][i] = (TH1D*) _f[f]->Get(Form("TimeProfile_pmtType%d",i));
            TimeProfile[f][i]->SetLineWidth(2);
            TimeProfile[f][i]->SetLineColor(colorScale[i]);
            TimeProfile[f][i]->GetXaxis()->SetTitle("Time (ns)");
            TimeProfile[f][i]->GetYaxis()->SetTitle("Charge (p.e) (normalized)");

            TimeTOFProfile[f][i] = (TH1D*) _f[f]->Get(Form("TimeTOFProfile_pmtType%d",i));
            TimeTOFProfile[f][i]->SetLineWidth(2);
            TimeTOFProfile[f][i]->SetLineColor(colorScale[i]);
            TimeTOFProfile[f][i]->GetXaxis()->SetTitle("Time - TOF (ns)");
            TimeTOFProfile[f][i]->GetYaxis()->SetTitle("Charge (p.e) (normalized)");

            HitTimeTOFProfile[f][i] = (TH1D*) _f[f]->Get(Form("HitTimeTOFProfile_pmtType%d",i));
            HitTimeTOFProfile[f][i]->SetLineWidth(2);
            HitTimeTOFProfile[f][i]->SetLineColor(colorScale[i]);
            HitTimeTOFProfile[f][i]->GetXaxis()->SetTitle("HitTime - TOF (ns)");
            HitTimeTOFProfile[f][i]->GetYaxis()->SetTitle("nhits (normalized)");

            HitTimeTOFDR[f][i] = (TH1D*)  HitTimeTOFProfile[f][i]->Clone(Form("HitTimeTOFDR_pmtType%d",i));
            HitTimeTOFDR[f][i]->Reset();


            TimeTOFProfileXTOF[f][i] = (TH2D*) _f[f]->Get(Form("TimeTOFProfileXTOF_pmtType%d",i));
            //TimeTOFProfileXTOF[f][i]->SetLineWidth(2);
            TimeTOFProfileXTOF[f][i]->GetXaxis()->SetTitle("Time - TOF (ns)");
            TimeTOFProfileXTOF[f][i]->GetYaxis()->SetTitle("TOF (ns)");

            if(isNew){
                angularResponseXPMT_hits[f][i] = (TH3D*) _f[f]->Get(Form("angularResponseXPMT_hits_pmtType%d",i));
                angularResponseXPMT_allPMTs[f][i] = (TH3D*) _f[f]->Get(Form("angularResponseXPMT_allPMTs_pmtType%d",i));
                //ChargeProfile2DXPMT[f][i] = (TH3D*) _f[f]->Get(Form("ChargeProfile2DXPMT_pmtType%d",i));
                cout<<"Ola"<<endl;
            }
        }

        if(isNew){
            const int nPMT = 4;
            const int nGroupsPMTs = 3;
            fOut->cd();
            TH2D *hProjection;
            TH2D *hProjection2;
            TH2D *angular2DResponse_hits[nPMTtypes][nPMT];
            for(int i=0;i<2;i++){
                if(f==0){
                    cAngularResponsePMT_hits = new TCanvas("cAngularResponsePMT_hits","cAngularResponsePMT_hits");
                    //nPMT = angularResponseXPMT_hits[f][i]->GetNbinsX();
                    cAngularResponsePMT_hits->Divide( floor(TMath::Sqrt(nPMT)) , floor(TMath::Sqrt(nPMT)) );
                }
                cout<<"npmt per mPMT="<<nPMT<<endl;

                for(int ibinx=1;ibinx<=nGroupsPMTs;ibinx++){
                    cAngularResponsePMT_hits->cd(ibinx);

                    //angularResponseXPMT_hits[f][i]->GetXaxis()->SetRange(ibinx,ibinx);
                    if(ibinx==1) angularResponseXPMT_hits[f][i]->GetXaxis()->SetRange(1,12);
                    else if(ibinx==2) angularResponseXPMT_hits[f][i]->GetXaxis()->SetRange(13,18);
                    else angularResponseXPMT_hits[f][i]->GetXaxis()->SetRange(19,19);

                    hProjection = (TH2D*) angularResponseXPMT_hits[f][i]->Project3D("zy");
                    //angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(ibinx,ibinx);
                    if(ibinx==1) angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(1,12);
                    else if(ibinx==2) angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(13,18);
                    else angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(19,19);
                    hProjection2 = (TH2D*) angularResponseXPMT_allPMTs[f][i]->Project3D("zy");
                    hProjection->Divide(hProjection2);

                    angular2DResponse_hits[i][ibinx-1] = (TH2D*) hProjection->Clone(Form("angular2DResponse_hits_pmtType%d_pmt%d",i,ibinx));
                    angular2DResponse_hits[i][ibinx-1]->Rebin2D(2,2);
                    angular2DResponse_hits[i][ibinx-1]->Draw("colz");
                    //angular2DResponse_hits[i][ibinx-1]->Write(Form("hPMTDirectionality_%d_%d_%d",f,i,ibinx-1));
                }
            }

            TH1D *hProjection_1D;
            TH1D *hProjection2_1D;
            TH1D *angular2DResponse_hits_1D[nPMTtypes][nPMT];
            for(int i=0;i<2;i++){
                if(f==0){
                    cAngularResponsePMT_hits_1D = new TCanvas("cAngularResponsePMT_hits_1D","cAngularResponsePMT_hits_1D");
                }
                cout<<"npmt per mPMT="<<nPMT<<endl;

                for(int ibinx=1;ibinx<=nGroupsPMTs;ibinx++){
                    angularResponseXPMT_hits[f][i]->GetXaxis()->SetRange(ibinx,ibinx);

                    hProjection_1D = (TH1D*) angularResponseXPMT_hits[f][i]->Project3D("z");
                    angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(ibinx,ibinx);
                    hProjection2_1D = (TH1D*) angularResponseXPMT_allPMTs[f][i]->Project3D("z");
                    hProjection_1D->Rebin(3);
                    hProjection2_1D->Rebin(3);
                    hProjection_1D->Divide(hProjection2_1D);

                    angular2DResponse_hits_1D[i][ibinx-1] = (TH1D*) hProjection_1D->Clone(Form("angular2DResponse_hits_1D_pmtType%d_pmt%d",i,ibinx));
                    //angular2DResponse_hits_1D[i][ibinx-1]->Rebin(3);
                    angular2DResponse_hits_1D[i][ibinx-1]->SetLineWidth(2);
                    angular2DResponse_hits_1D[i][ibinx-1]->SetLineColor(colorScale2[ibinx-1]);
                    if(ibinx==1) angular2DResponse_hits_1D[i][ibinx-1]->Draw();
                    else angular2DResponse_hits_1D[i][ibinx-1]->Draw("same");
                    //angular2DResponse_hits_1D[i][ibinx-1]->Scale(1./angular2DResponse_hits_1D[i][ibinx-1]->Integral());
                    angular2DResponse_hits_1D[i][ibinx-1]->Write(Form("hPMTDirectionality_1D_%d_%d_%d",f,i,ibinx-1));
                }
            }




            TH2D *angular2DResponse_allPMTs[nPMTtypes][nPMT];
            for(int i=1;i<2;i++){
                if(f==0){
                    cAngularResponsePMT_allPMTs = new TCanvas("cAngularResponsePMT_allPMTs","cAngularResponsePMT_allPMTs");
                    //nPMT = angularResponseXPMT_allPMTs[f][i]->GetNbinsX();
                    cAngularResponsePMT_allPMTs->Divide( floor(TMath::Sqrt(nPMT)) , floor(TMath::Sqrt(nPMT)) );
                }
                cout<<"npmt per mPMT="<<nPMT<<endl;

                for(int ibinx=1;ibinx<=nPMT;ibinx++){
                    cAngularResponsePMT_allPMTs->cd(ibinx);
                    angularResponseXPMT_allPMTs[f][i]->GetXaxis()->SetRange(ibinx,ibinx);
                    hProjection = (TH2D*) angularResponseXPMT_allPMTs[f][i]->Project3D("zy");
                    angular2DResponse_allPMTs[i][ibinx-1] = (TH2D*) hProjection->Clone(Form("angular2DResponse_allPMTs_pmtType%d_pmt%d",i,ibinx));
                    angular2DResponse_allPMTs[i][ibinx-1]->Rebin2D(2,2);
                    angular2DResponse_allPMTs[i][ibinx-1]->Draw("colz");
                }
            }

        }


        if(f==0) cChargePerPMT = new TCanvas("cChargePerPMT","cChargePerPMT");
        for(int i=0;i<nPMTtypes;i++){
            ChargePerPMT[f][i]->Scale(1./ChargePerPMT[f][i]->Integral());
            if(i==0){
                ChargePerPMT[f][i]->Draw("hist");
                ChargePerPMT[f][i]->GetXaxis()->SetRangeUser(0,8);
                ChargePerPMT[f][i]->GetYaxis()->SetRangeUser(0,1.);
            }
            else  ChargePerPMT[f][i]->Draw("histsame");
            l->Draw("same");
        }
        cChargePerPMT->SaveAs(Form("plots/%s.eps",cChargePerPMT->GetName()));


        if(f==0){
            cChargeProfile = new TCanvas("cChargeProfile","cChargeProfile");
            cChargeProfile->Divide(1,2);
        }
        for(int i=0;i<nPMTtypes;i++){
            ChargeProfile[f][i]->Scale(1/nEvents);
            ChargeProfile[f][i]->Rebin(4);

            ratioChargeProfile[f][i] = (TH1D*) ChargeProfile[f][i]->Clone(Form("ratioChargeProfile_%d_%d",f,i));
            ratioChargeProfile[f][i]->Divide(ChargeProfile[f][0]);

            if(i==0){
                cChargeProfile->cd(1);
                ChargeProfile[f][i]->Draw("hist");
                //ChargeProfile[f][i]->GetXaxis()->SetRangeUser(0,8);
                //ChargeProfile[f][i]->GetYaxis()->SetRangeUser(0,1.);
            }
            else{
                cChargeProfile->cd(1);
                ChargeProfile[f][i]->Draw("histsame");
                l->Draw("same");
                if(i==1){
                    cChargeProfile->cd(2);
                    ratioChargeProfile[f][i]->Draw("hist");
                }
                else{
                    cChargeProfile->cd(2);
                    ratioChargeProfile[f][i]->Draw("histsame");
                    l->Draw("same");
                }
            }
        }
        cChargeProfile->SaveAs(Form("plots/%s.eps",cChargeProfile->GetName()));


        if(isNew2){
            if(f==0){
                cChargeProfileXdWall = new TCanvas("cChargeProfileXdWall","cChargeProfileXdWall");
                cChargeProfileXdWall->Divide(1,2);
            }
            for(int i=0;i<nPMTtypes;i++){
                ChargeProfileXdWall[f][i]->Scale(1/nEvents);
                ChargeProfileXdWall[f][i]->Rebin2D(100,4);

                cChargeProfileXdWall->cd(i+1);
                ChargeProfileXdWall[f][i]->Draw("colz");
            }
            cChargeProfileXdWall->SaveAs(Form("plots/%s.eps",cChargeProfileXdWall->GetName()));
        }

        if(f==0) cTotalCharge = new TCanvas("cTotalCharge","cTotalCharge");
        for(int i=0;i<nPMTtypes;i++){
            TotalCharge[f][i]->Scale(1./TotalCharge[f][i]->Integral());
            if(i==0){
                TotalCharge[f][i]->Draw("hist");
                TotalCharge[f][i]->GetXaxis()->SetRangeUser(0,5e3);
                TotalCharge[f][i]->GetYaxis()->SetRangeUser(0,0.6);
            }
            else  TotalCharge[f][i]->Draw("histsame");
            l->Draw("same");
        }
        cTotalCharge->SaveAs(Form("plots/%s.eps",cTotalCharge->GetName()));


        if(f==0) cTotalHit = new TCanvas("cTotalHit","cTotalHit");
        for(int i=0;i<nPMTtypes;i++){
            TotalHit[f][i]->Scale(1./TotalHit[f][i]->Integral());
            if(i==0){
                TotalHit[f][i]->Draw("hist");
                TotalHit[f][i]->GetXaxis()->SetRangeUser(0,5e3);
                TotalHit[f][i]->GetYaxis()->SetRangeUser(0,0.6);
            }
            else  TotalHit[f][i]->Draw("histsame");
            l->Draw("same");
        }
        cTotalHit->SaveAs(Form("plots/%s.eps",cTotalHit->GetName()));


        if(f==0) cTimeProfile = new TCanvas("cTimeProfile","cTimeProfile");
        for(int i=0;i<nPMTtypes;i++){
            TimeProfile[f][i]->Scale(1/TimeProfile[f][i]->Integral());
            //TimeProfile[f][i]->Scale(1/nEvents);
            TimeProfile[f][i]->Rebin(4);
            if(i==0){
                TimeProfile[f][i]->Draw("hist");
                TimeProfile[f][i]->GetXaxis()->SetRangeUser(0,600);
                //TimeProfile[f][i]->GetYaxis()->SetRangeUser(0,1.);
            }
            else  TimeProfile[f][i]->Draw("histsame");
            l->Draw("same");
        }
        cTimeProfile->SaveAs(Form("plots/%s.eps",cTimeProfile->GetName()));



        //Very important here: let profile and DR have same transformation (same binning, same rescaling etc) as they should be kept in the same proportions as signal
        if(f==0) cTimeTOFProfile = new TCanvas("cTimeTOFProfile","cTimeTOFProfile");
        for(int i=0;i<nPMTtypes;i++){
            //We will zoom in the time tof profile region from -100 to -20ns.

            TimeTOFProfile[f][i]->Rebin(1);
            //cout<<"Integral signal from -100 to 500ns = "<<TimeTOFProfile[f][i]->Integral(TimeTOFProfile[f][i]->FindBin(-100),TimeTOFProfile[f][i]->FindBin(500))<<", DR="<<TimeTOFDR[f][i]->Integral(TimeTOFDR[f][i]->FindBin(-100),TimeTOFDR[f][i]->FindBin(500))<<endl;

            double scaleValue=events[i]/2;//TimeTOFProfile[f][i]->GetBinContent(TimeTOFProfile[f][i]->GetMaximumBin());
            TimeTOFProfile[f][i]->Scale(1/scaleValue);
            if(i==0){
                TimeTOFProfile[f][i]->Draw("hist");
                TimeTOFProfile[f][i]->GetXaxis()->SetRangeUser(-10,30);
                //TimeTOFProfile[f][i]->GetYaxis()->SetRangeUser(0,1.);
            }
            else TimeTOFProfile[f][i]->Draw("histsame");
        }   
        ///////////////////////////////////////

        //Very important here: let profile and DR have same transformation (same binning, same rescaling etc) as they should be kept in the same proportions as signal
        if(f==0) cHitTimeTOFProfile = new TCanvas("cHitTimeTOFProfile","cHitTimeTOFProfile");
        for(int i=0;i<nPMTtypes;i++){
            //We will zoom in the time tof profile region from -100 to -20ns.
            double startDR=-100;
            double endDR=-20;
            double timeWindowDR=endDR-startDR;
            DRTotalPerNS[i]=HitTimeTOFProfile[f][i]->Integral(HitTimeTOFProfile[f][i]->FindBin(startDR),HitTimeTOFProfile[f][i]->FindBin(endDR));
            DRTotalPerNS[i]/=timeWindowDR;

            for(int ibinx=1;ibinx<= HitTimeTOFProfile[f][i]->GetNbinsX();ibinx++){
                double timeWindow=HitTimeTOFProfile[f][i]->GetBinWidth(ibinx);
                HitTimeTOFDR[f][i]->SetBinContent(ibinx,DRTotalPerNS[i]*timeWindow);
                //cout<<HitTimeTOFDR[f][i]->GetBinCenter(ibinx)<<", time window="<<timeWindow<<", signal="<<HitTimeTOFProfile[f][i]->GetBinContent(ibinx)<<", DR="<<HitTimeTOFDR[f][i]->GetBinContent(ibinx)<<", bin width="<<timeWindow<<", DR="<<DRTotalPerNS[i]<<", nevents="<<events[i]/2<<endl;
            }
            HitTimeTOFProfile[f][i]->Rebin(1);
            HitTimeTOFDR[f][i]->Rebin(1);
            //cout<<"Integral signal from -100 to 500ns = "<<HitTimeTOFProfile[f][i]->Integral(HitTimeTOFProfile[f][i]->FindBin(-100),HitTimeTOFProfile[f][i]->FindBin(500))<<", DR="<<HitTimeTOFDR[f][i]->Integral(HitTimeTOFDR[f][i]->FindBin(-100),HitTimeTOFDR[f][i]->FindBin(500))<<endl;

            double scaleValue=events[i]/2;//HitTimeTOFProfile[f][i]->GetBinContent(HitTimeTOFProfile[f][i]->GetMaximumBin());
            HitTimeTOFProfile[f][i]->Scale(1/scaleValue);
            HitTimeTOFDR[f][i]->Scale(1/scaleValue);
            //HitTimeTOFProfile[f][i]->Scale(1/nEvents);
            if(i==0){
                HitTimeTOFProfile[f][i]->Draw("hist");
                HitTimeTOFProfile[f][i]->GetXaxis()->SetRangeUser(-10,30);
                //HitTimeTOFProfile[f][i]->GetYaxis()->SetRangeUser(0,1.);
            }
            else HitTimeTOFProfile[f][i]->Draw("histsame");
            HitTimeTOFDR[f][i]->SetLineColor(kMagenta);
            HitTimeTOFDR[f][i]->Draw("histsame");
            cout<<"Drawn"<<endl;
            cout<<"Integral signal from -100 to 500ns = "<<HitTimeTOFProfile[f][i]->Integral(HitTimeTOFProfile[f][i]->FindBin(-100),HitTimeTOFProfile[f][i]->FindBin(500))<<", DR="<<HitTimeTOFDR[f][i]->Integral(HitTimeTOFDR[f][i]->FindBin(-100),HitTimeTOFDR[f][i]->FindBin(500))<<endl;

            ///////////////////////////////////////

            gausExpoConv[f][i] = new TF1(Form("gausExpoConv%d_%d",f,i),".5*[0]*TMath::Exp(-[3]*((x-[1])-[2]*[2]*[3]/2))*(1 + TMath::Erf( ((x-[1])-[2]*[2]*[3])/(TMath::Sqrt(2)*[2])))",-2.,limitFitGausExpo[i]);
            gausExpoConv2[f][i] = new TF1(Form("gausExpoConv2%d_%d",f,i),"[0]*(TMath::Exp(-x/[3])*(1 + TMath::Erf( ((x-[1])-[2]*[2]/[3])/(TMath::Sqrt(2)*[2]))) + [4]*TMath::Exp(-x/[5])*(1 + TMath::Erf( ((x-[1])-[2]*[2]/[5])/(TMath::Sqrt(2)*[2]))))",-2,limitFitGausExpo[i]); // 2 component of exponential convoluted with gaussian
            gausExpoConv[f][i]->SetLineColor(kMagenta);
            gausExpoConv[f][i]->SetLineWidth(2);
            gausExpoConv[f][i]->SetParameter(0,1.);
            gausExpoConv[f][i]->SetParameter(1,-1.);
            gausExpoConv[f][i]->SetParameter(2,0.7);
            gausExpoConv[f][i]->SetParameter(3,0.5);
            HitTimeTOFProfile[f][i]->Fit(Form("gausExpoConv%d_%d",f,i),"", "", -2., 100.);
            gausExpoConv2[f][i]->SetLineColor(kOrange);
            gausExpoConv2[f][i]->SetLineWidth(2);
            gausExpoConv2[f][i]->SetParameter(0,gausExpoConv[f][i]->GetParameter(0)*0.5);
            gausExpoConv2[f][i]->SetParameter(1,gausExpoConv[f][i]->GetParameter(1));
            gausExpoConv2[f][i]->SetParameter(2,gausExpoConv[f][i]->GetParameter(2));
            gausExpoConv2[f][i]->SetParameter(3,1./gausExpoConv[f][i]->GetParameter(3));
            gausExpoConv2[f][i]->SetParameter(4,0.01);
            gausExpoConv2[f][i]->SetParLimits(4,0., 0.5);
            gausExpoConv2[f][i]->SetParameter(5,100./gausExpoConv[f][i]->GetParameter(3));
            HitTimeTOFProfile[f][i]->Fit(Form("gausExpoConv2%d_%d",f,i),"", "", -2., 100.);
            HitTimeTOFProfile[f][i]->Fit(Form("gausExpoConv2%d_%d",f,i),"M", "", -2., 100.);
            gausExpoConv[f][i]->Draw("same");
            gausExpoConv2[f][i]->Draw("same");
            l->Draw("same");

            splineExpoConv[f][i] = new TSpline3(Form("splineExpoConv%d_%d",f,i),-limitFitGausExpo[i],limitFitGausExpo[i],gausExpoConv[f][i],3000);
            splineExpoConv2[f][i] = new TSpline3(Form("splineExpoConv2%d_%d",f,i),-limitFitGausExpo[i],limitFitGausExpo[i],gausExpoConv2[f][i],3000);
            splineExpoConv[f][i]->SetLineColor(kCyan);
            splineExpoConv2[f][i]->SetLineColor(kCyan);
            //splineExpoConv[f][i]->Draw("lcsame");
            fOut->cd();
            splineExpoConv[f][i]->Write(splineExpoConv[f][i]->GetTitle());
            splineExpoConv2[f][i]->Write(splineExpoConv2[f][i]->GetTitle());

            const int nBins=270;//150;//Number of big bins
            double xPos[nBins];
            double yPos[nBins];
            double drPos[nBins];
            int iBinActive=0;
            int nRebins=10;//Number of small bin gathered in a big one
            double xAverage=0;
            double yAverage=0;
            double drAverage=0;
            int binLimit=HitTimeTOFProfile[f][i]->FindBin(-limitFitGausExpo[i]); //Lower limit of the active big bin


            for(int ibinx=HitTimeTOFProfile[f][i]->FindBin(-limitFitGausExpo[i]);ibinx<=HitTimeTOFProfile[f][i]->GetNbinsX();ibinx++){
                xAverage+=HitTimeTOFProfile[f][i]->GetBinCenter(ibinx);
                yAverage+=HitTimeTOFProfile[f][i]->GetBinContent(ibinx);
                drAverage+=HitTimeTOFDR[f][i]->GetBinContent(ibinx);

                //if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<-3) nRebins=1;
                if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<-100) nRebins=100;
                else if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<-30) nRebins=10;
                else if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<-10) nRebins=5;
                else if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<8) nRebins=2;
                else if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<30) nRebins=10;
                else if(HitTimeTOFProfile[f][i]->GetBinCenter(ibinx)<100) nRebins=30;
                else nRebins =100;
                //cout<<"Rebins="<<nRebins<<", bin limit="<<ibinx-binLimit<<", average x="<<xAverage<<endl;
                if((ibinx-binLimit)%nRebins==(nRebins-1)){
                    xPos[iBinActive]=xAverage/nRebins;
                    drPos[iBinActive]=drAverage/nRebins;
                    yPos[iBinActive]=(yAverage/nRebins);
                    cout<<"Position = "<<xPos[iBinActive]<<", value="<<yPos[iBinActive]<<", DR = "<<drPos[iBinActive]<<endl;
                    iBinActive++;
                    xAverage=0;
                    yAverage=0;
                    drAverage=0;
                    binLimit=ibinx+1;
                }
                if(iBinActive>=nBins){
                    cout<<"Add more bins"<<endl;
                    break;
                }
            }

            graphExpoQueue[f][i] = new TGraph(nBins,xPos,yPos);

            splineExpoQueue[f][i] = new TSpline3(Form("splineExpoQueue%d_%d",f,i),graphExpoQueue[f][i]);
            splineExpoQueue[f][i]->SetLineColor(kCyan);
            splineExpoQueue[f][i]->Draw("lcsame");
            fOut->cd();
            splineExpoQueue[f][i]->Write(splineExpoQueue[f][i]->GetTitle());


            graphDR[f][i] = new TGraph(nBins,xPos,drPos);
            splineDR[f][i] = new TSpline3(Form("splineDR%d_%d",f,i),graphDR[f][i]);
            splineDR[f][i]->SetLineColor(kCyan);
            splineDR[f][i]->Draw("lcsame");
            fOut->cd();
            splineDR[f][i]->Write(splineDR[f][i]->GetTitle());

            cout<<"Integral signal from -100 to 500ns = "<<HitTimeTOFProfile[f][i]->Integral(HitTimeTOFProfile[f][i]->FindBin(-100),HitTimeTOFProfile[f][i]->FindBin(500))<<", DR="<<HitTimeTOFDR[f][i]->Integral(HitTimeTOFDR[f][i]->FindBin(-100),HitTimeTOFDR[f][i]->FindBin(500))<<endl;
            double stepSize=0.5;
            double integralSignal=0;
            double integralDR=0;
            for(double j=-100;j<500;j+=stepSize){
                integralSignal+=stepSize*splineExpoQueue[f][i]->Eval(j+stepSize/2);
                integralDR+=stepSize*splineDR[f][i]->Eval(j+stepSize/2);
            }
            cout<<"Integral spline="<<integralSignal<<", DR="<<integralDR<<endl;
        }
        cHitTimeTOFProfile->SaveAs(Form("plots/%s.eps",cHitTimeTOFProfile->GetName()));

        if(f==0){
            cTimeTOFProfileXTOF = new TCanvas("cTimeTOFProfileXTOF","cTimeTOFProfileXTOF");
            cTimeTOFProfileXTOF->Divide(1,2);

            for(int i=0;i<nPMTtypes;i++){
                TimeTOFProfileXTOF[f][i]->Scale(1/TimeTOFProfileXTOF[f][i]->Integral());
                TimeTOFProfileXTOF[f][i]->Rebin2D(2,4);
                cTimeTOFProfileXTOF->cd(i+1);
                TimeTOFProfileXTOF[f][i]->Draw("colz");
                TimeTOFProfileXTOF[f][i]->GetXaxis()->SetRangeUser(-50,100);
                TimeTOFProfileXTOF[f][i]->GetYaxis()->SetRangeUser(0,300);
            }
            cTimeTOFProfileXTOF->SaveAs(Form("plots/%s.eps",cTimeTOFProfileXTOF->GetName()));
        }


        if(f==0){
            cTimeTOFProfileXTOF_1D = new TCanvas("cTimeTOFProfileXTOF_1D","cTimeTOFProfileXTOF_1D");
            cTimeTOFProfileXTOF_1D->Divide( floor(TMath::Sqrt(nBinsTOF)) , floor(TMath::Sqrt(nBinsTOF)) );

            for(int i=0;i<nPMTtypes;i++){

                for(int ibiny=1;ibiny<=nBinsTOF;ibiny++){

                    TimeTOFProfileXTOF_1D[f][i][ibiny-1] = (TH1D*) TimeTOFProfileXTOF[f][i]->ProjectionX(Form("TimeTOFProfileXTOF%d_%d_%d",f,i,ibiny),ibiny,ibiny);
                    cTimeTOFProfileXTOF_1D->cd(ibiny);
                    TimeTOFProfileXTOF_1D[f][i][ibiny-1]->SetLineColor(colorScale[i]);

                    if(i==0){
                        TimeTOFProfileXTOF_1D[f][i][ibiny-1]->Draw("hist");
                        TimeTOFProfileXTOF_1D[f][i][ibiny-1]->GetXaxis()->SetRangeUser(-5,10);
                    }
                    else  TimeTOFProfileXTOF_1D[f][i][ibiny-1]->Draw("histsame");
                    l->Draw("same");

                    gausExpoConv_slice[f][i][ibiny-1] = new TF1(Form("gausExpoConv_slice%d_%d_%d",f,i,ibiny-1),".5*[0]*TMath::Exp(-[3]*((x-[1])-[2]*[2]*[3]/2))*(1 + TMath::Erf( ((x-[1])-[2]*[2]*[3])/(TMath::Sqrt(2)*[2])))",-3,10);
                    gausExpoConv_slice[f][i][ibiny-1]->SetLineColor(1);
                    gausExpoConv_slice[f][i][ibiny-1]->SetLineWidth(2);
                    gausExpoConv_slice[f][i][ibiny-1]->SetParameter(0,10.);
                    gausExpoConv_slice[f][i][ibiny-1]->SetParameter(1,0.);
                    gausExpoConv_slice[f][i][ibiny-1]->SetParameter(2,1.);
                    gausExpoConv_slice[f][i][ibiny-1]->SetParameter(3,2.);

                    //TimeTOFProfileXTOF_1D[f][i][ibiny-1]->Fit(Form("gausExpoConv_slice%d_%d_%d",f,i,ibiny-1),"R");
                    gausExpoConv_slice[f][i][ibiny-1]->Draw("same");
                }
            }
        }
    }

    fOut->Close();
    return;
}

