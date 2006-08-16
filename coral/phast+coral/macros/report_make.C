void report_make(void)
{
    gROOT->LoadMacro("report.C");
    gROOT->LoadMacro("Albert-residuals.C");
    RS.create_out("out.root");
    
    Albert_residuals();
}
