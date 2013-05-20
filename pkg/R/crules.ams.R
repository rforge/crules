crules.ams <- function(formula, trainingSet, qPruneSet = crules.qPruneSet, 
					   qGrowSet = crules.qGrowSet, criterion, folds = 10, byPair = FALSE) {
	
	if(criterion == "acc")
		critIndex = 1
	else if(criterion == "bac")
		critIndex = 2
	else if(criterion == "g-mean"){
		critIndex = "g"
		mf <- model.frame(formula, trainingSet, na.action=na.pass)
		y <- mf[,1, drop=TRUE]
		if(length(levels(factor(y))) > 2)
			stop("G-mean criterion is not supported for multi-class data sets.")
	}
	else
		stop("Bad criterion! Allowed values: \"acc\", \"bac\", \"g-mean\"")
	
	useCV <- TRUE
	
	if(folds > 0 && folds < 1){
		trLength <- round(nrow(trainingSet) * folds)
		trNumbers <- sample(nrow(trainingSet), trLength)
		trSet <- trainingSet[trNumbers,]
		valSet <- trainingSet[-trNumbers,]
		useCV <- FALSE
	}

	bestCrit <- -Inf
	bestQPrune <- NA
	bestQGrow <- NA
	
	computeAndUpdate <- function(qPrunePar, qGrowPar){
		if(useCV){
			cv <- crules.cv(formula, trainingSet, qPrunePar, qGrowPar, folds = folds)
			if(critIndex != "g")
				currCrit <- mean(cv)$Statistics[critIndex, "Mean"]
			else
				currCrit <- .computeGmeanFromConfusionMatrix(mean(cv)$"Average confusion matrix")		
		}
		else{
			rules <- crules(formula, trSet, qPrunePar, qGrowPar)
			res <- predict(rules, valSet)
			if(critIndex != "g")
				currCrit <- res[[critIndex]]
			else
				currCrit <- .computeGmeanFromConfusionMatrix(res$confusionMatrix)
		}
		
		if (currCrit > bestCrit) {
			bestCrit <<- currCrit
			bestQPrune <<- qPrunePar
			bestQGrow <<- qGrowPar
		}
	}
	
	if(!byPair){	
		for (qPrune in qPruneSet) {
			for (qGrow in qGrowSet) {
				computeAndUpdate(qPrune, qGrow)
			}
		}
	}
	else {
		if(length(qPruneSet) != length(qGrowSet))
			stop("Vectors of rule quality measures' names are not of equal length!")
		
		for(i in seq_along(length(qPruneSet))){
			computeAndUpdate(qPruneSet[i], qGrowSet[i])
		}
	}
	
	BestCriterions <- c(bestQPrune, bestQGrow)
	names(BestCriterions) <- c("Growth", "Pruning")
	
	list(Rules = crules(formula, trainingSet, bestQPrune, bestQGrow), 
		 BestCriterions = BestCriterions)
} 

.computeGmeanFromConfusionMatrix <- function(cm){
	accMinus <- cm[2,2]/(cm[2,2]+cm[2,1])
	accPlus <- cm[1,1]/(cm[1,1]+cm[1,2])
	sqrt(accMinus * accPlus)
}

#crules.ams(Species~., iris, c("rss", "c2"), c("ls", "corr"), "acc", folds=0.6, byPair=TRUE)