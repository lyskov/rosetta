# -*- tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
# vi: set ts=2 noet:
#
# (c) Copyright Rosetta Commons Member Institutions.
# (c) This file is part of the Rosetta software suite and is made available under license.
# (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
# (c) For more information, see http://www.rosettacommons.org. Questions about this can be
# (c) addressed to University of Washington CoMotion, email: license@uw.edu.

check_setup()

feature_analyses <- c(feature_analyses, new("FeaturesAnalysis",
id = "int_packing-packstat_vs",
author = "Jared Adolf-Bryfogle",
brief_description = "Graphs Interface metrics such as packstat and shape complementarity scores",
feature_reporter_dependencies = c("InterfaceFeatures/AntibodyFeatures"),
run=function(self, sample_sources, output_dir, output_formats){
  
  sele <- "
    SELECT
      packstat,
      dSASA,
      delta_unsatHbonds,
      interface
    FROM
      interfaces
  "
  
  plot_field = function(p, plot_id, grid = NULL){
    
    if (! is.null(grid)){
      p <- p+ facet_grid(facets=grid)
    }
    if(nrow(sample_sources) <= 3){
      p <- p + theme(legend.position="bottom", legend.direction="horizontal")
    }
    save_plots(self, plot_id, sample_sources, output_dir, output_formats)
  }
  
  plot_parts <- list(
    geom_indicator(aes(indicator=counts, colour=sample_source, group=sample_source)),
    scale_y_continuous("Feature Density"),
    theme_bw())
  
  
  data = query_sample_sources(sample_sources, sele)


  #Scatterplots
  #sc_value vs packstat
  parts = list(
    geom_point(size=1.5, pch="o"),
    #stat_smooth(color="grey"),
    stat_smooth(method=lm),
    geom_density2d(),
    #stat_density2d(aes(fill = ..level..), geom="polygon"),
    #stat_density2d(geom="tile", aes(fill = ..density..), contour = FALSE),
    theme_bw())
  
  #packstat vs dSASA
  p <- ggplot(data = data, aes(x=packstat, y=dSASA)) + parts +
    ggtitle("packstat vs dSASA") +
    scale_x_continuous("packstat", limit=c(0,1.0)) +
    scale_y_continuous("Buried SASA")
  plot_field(p, "packstat_vs_dSASA_by_all", grid = sample_source ~.)
  plot_field(p, "packstat_vs_dSASA_by_interface", grid = interface ~ sample_source)
  
  
  #deltaUnsatHbonds vs packstat
  p <- ggplot(data = data, aes(x = delta_unsatHbonds, y=packstat)) + parts +
    ggtitle("packstat vs interface unsatisfied polar atoms ") +
    scale_x_continuous("n") +
    scale_y_continuous("packstat", limit = c(0, 1.0))
  plot_field(p, "packstat_vs_delta_unsat_polars_by_all", grid=sample_source ~ .)
  plot_field(p, "packstat_vs_delta_unsat_polars_by_interface", grid = interface ~ sample_source)
  #3D Plots
  
  #sc_value vs dG vs dSASA
  
  #Sides:
  
  #sc_value vs interface_energy
  
})) # end FeaturesAnalysis