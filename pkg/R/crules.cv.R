setClass("crules.cv", representation(results = "list"))

crules.cv <- function(formula, data, q, qsplit=q, folds=10, runs=1, 
		everyClassInFold = TRUE, weights, useWeightsInPrediction = TRUE)
{
	if(runs <= 0 || folds <= 1 || folds > nrow(data))
		stop("Incorrect number of folds or runs")
	params <- .prepare.data(formula, data, q, qsplit, weights)
	params <- c(params, folds = folds, runs = runs, everyClassInFold = everyClassInFold, 
				useWeightsInPrediction = useWeightsInPrediction)
	
	rarc <- new( RInterface)
	result <- rarc$crossValidation(params)
	rm(rarc)
	
	size <- folds * runs
	rules <- vector("list", size)
	results <- vector("list", size)
	for(run in 1:runs){
		for(fold in 1:folds){
			result[[run]][[fold]][[1]] <- new("crules", rules = result[[run]][[fold]][[1]],  
					yname = params$yname, ylevels = params$ylevels, xnames = params$xnames,
					xlevels = params$xlevels, xtypes = params$xtypes, call = match.call(expand.dots = FALSE))
			result[[run]][[fold]][[2]] <- .prep.pred.res(result[[run]][[fold]][[2]], 
					result[[run]][[fold]][[1]]@ylevels)
		}
	}
	new("crules.cv", results = result)
}

setMethod("mean", "crules.cv", function(x){
			#Means:
			mean <- vector("numeric", 8)
			size <- 0
			cm <- x@results[[1]][[1]][[2]]$confusionMatrix * 0
			for(run in 1:length(x@results)){
				for(fold in 1:length(x@results[[run]])){
					size <- size + 1
					mean[1] <- mean[1] + x@results[[run]][[fold]][[2]]$acc
					mean[2] <- mean[2] + x@results[[run]][[fold]][[2]]$bac
					mean[3] <- mean[3] + x@results[[run]][[fold]][[2]]$cov
					mean[4] <- mean[4] + length(x@results[[run]][[fold]][[1]]@rules$NumbersOfConditions)
					mean[5] <- mean[5] + mean(x@results[[run]][[fold]][[1]]@rules$NumbersOfConditions)
					mean[6] <- mean[6] + mean(x@results[[run]][[fold]][[1]]@rules$RulesPrecisions)
					mean[7] <- mean[7] + mean(x@results[[run]][[fold]][[1]]@rules$RulesCoverages)
					mean[8] <- mean[8] + mean(x@results[[run]][[fold]][[1]]@rules$Pvalues)
					cm <- cm + x@results[[run]][[fold]][[2]]$confusionMatrix
				}
			}
			for(i in 1:length(mean))
				mean[i] <- mean[i] / size
			cm <- cm / size
			#std deviations:
			stddev <- vector("numeric", 8)
			for(run in 1:length(x@results)){
				for(fold in 1:length(x@results[[run]])){
					stddev[1] <- stddev[1] + (x@results[[run]][[fold]][[2]]$acc - mean[1])^2
					stddev[2] <- stddev[2] + (x@results[[run]][[fold]][[2]]$bac - mean[2])^2
					stddev[3] <- stddev[3] + (x@results[[run]][[fold]][[2]]$cov - mean[3])^2
					stddev[4] <- stddev[4] + (length(x@results[[run]][[fold]][[1]]@rules$NumbersOfConditions) - mean[4])^2
					stddev[5] <- stddev[5] + (mean(x@results[[run]][[fold]][[1]]@rules$NumbersOfConditions) - mean[5])^2
					stddev[6] <- stddev[6] + (mean(x@results[[run]][[fold]][[1]]@rules$RulesPrecisions) - mean[6])^2
					stddev[7] <- stddev[7] + (mean(x@results[[run]][[fold]][[1]]@rules$RulesCoverages)- mean[7])^2
					stddev[8] <- stddev[8] + (mean(x@results[[run]][[fold]][[1]]@rules$Pvalues)- mean[8])^2
				}
			}
			for(i in 1:length(stddev))
				stddev[i] <- sqrt(stddev[i] / size)
			
			result <- data.frame(mean, stddev, row.names = c("Accuracy:", 
							"Class accuracy:",
							"Coverage:",
							"Number of rules:", 
							"Average number of elementary conditions per rule:",
							"Average precision of rules:",
							"Average coverage of rules:",
							"Average p-value of rules"))
			names(result) <- c("Mean", "Std deviation")
			list(Statistics = result, "Average confusion matrix" = cm)
		})

setMethod("as.data.frame", "crules.cv", function(x, dataname, qname, qsplitname){
			size <- length(x@results)
			if(size > 0) 
				size <- length(x@results[[1]]) * size
			else stop("Cannot use results of 0 runs")
			if(missing(dataname))
				dataname <- as.character(x@results[[1]][[1]][[1]]@call["data"])
			if(missing(qname))
				qname <- as.character(x@results[[1]][[1]][[1]]@call["q"])
			if(missing(qsplitname)){
				qsplitname <- as.character(x@results[[1]][[1]][[1]]@call["qsplit"])
				if(qsplitname == "NULL") qsplitname <- qname
			}
			dataset <- rep(dataname, size)
			experiment <- rep(paste(qsplitname, qname, sep="-"), size)
			runs <- vector("integer", size)
			iteration <- vector("integer", size)
			acc <- vector("double", size)
			avg_acc <- vector("double", size)
			cov <- vector("double", size)
			rulesCount <- vector("integer", size)
			avg_conditions <- vector("double", size)
			avg_rule_acc <- vector("double", size)
			avg_rule_cov <- vector("double", size)
			num <- 0
			for(run in 1:length(x@results)){
				for(fold in 1:length(x@results[[run]])){
					num <- num + 1
					runs[num] <- run
					iteration[num] <- fold
					acc[num] <- x@results[[run]][[fold]][[2]]$acc
					avg_acc[num] <- x@results[[run]][[fold]][[2]]$bac
					cov[num] <- x@results[[run]][[fold]][[2]]$cov
					rulesCount[num] <- length(x@results[[run]][[fold]][[1]]@rules$Rules)
					avg_conditions[num] <- mean(x@results[[run]][[fold]][[1]]@rules$NumbersOfConditions)
					avg_rule_acc[num] <- mean(x@results[[run]][[fold]][[1]]@rules$RulesPrecisions)
					avg_rule_cov[num] <- mean(x@results[[run]][[fold]][[1]]@rules$RulesCoverages)
				}
			}
			resFrame <- data.frame(dataset, experiment, runs,iteration,acc,avg_acc,cov,
					rulesCount,avg_conditions,avg_rule_acc,avg_rule_cov)
			names(resFrame) <- c("dataset", "experiment", "run","iteration", 
					"%acc", "%bac", "%cov", "#rules", "#avg_conditions", 
					"avg_rule_acc", "avg_rule_cov")
			resFrame
		})