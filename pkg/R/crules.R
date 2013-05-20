setClass("crules", representation(rules = "list",  yname = "character",
				ylevels = "character", xnames = "character",
				xlevels = "list", xtypes = "character",
				call = "call"))


.crules.print <- function(x){
	print(data.frame(Rule = x@rules$Rules, Precision = x@rules$RulesPrecisions, 
					Coverage = x@rules$RulesCoverages, "P-value" = x@rules$Pvalues), 
			right = FALSE)
}

setMethod("print", "crules", .crules.print)
setMethod("show", "crules", .crules.print)

crules.qPruneSet <- c("g2", "lift", "ls", "rss", "corr", "s", "c1", "c2", "cn2", "gain")
crules.qGrowSet <- c(crules.qPruneSet, "entropy")

.prepare.data <- function(formula, data, q, qsplit, weights){

	if(is.character(q)){	
		q <- match.arg(q, crules.qPruneSet)
		qfun <- NULL
	}
	else if(is.function(q)){
		qfun <- q
		qsplit  #forcing the promise
		q <- ""
	}
	if(is.character(qsplit)){
		qsplit <- match.arg(qsplit, crules.qGrowSet)
		qsplitfun <- NULL
	}
	else if(is.function(qsplit)){
		qsplitfun <- qsplit
		qsplit <- ""
	}
	
	#data evaluation
	mf <- model.frame(formula, data, na.action=na.pass)
	y <- mf[,1, drop=TRUE]
	if(is.factor(y) || is.character(y) || is.logical(y)){
		y <- as.factor(y)
	}
	else
		stop("Decision attribute can only be of factor, character or logical type")
	
	x <- mf[,-1, drop=FALSE]
	
	#weights:
	weights <- .check.weights(weights, nrow(data))
	
	#levels, names, types
	yname <- names(mf[,1, drop=FALSE])
	ylevels <- levels(y)
	xdata <- .prepare.xdata(x)
	
	list(y = y, yname = yname, ylevels = ylevels, x = x, xtypes = xdata$xtypes, xnames = xdata$xnames,
			xlevels = xdata$xlevels, q = q, qsplit = qsplit, qfun = qfun, qsplitfun = qsplitfun,
			weights = weights, seed = runif(1))
}

.check.weights <- function(weights, n){
	if(missing(weights))
		return(vector("numeric"))
	else if(length(weights) != n)
		stop("weights vector should be the same length as the number of examples")
	else if(any(weights <= 0))
		stop("weights cannot be negative numbers")
}

.prepare.xdata <- function(x){
	xncol <- ncol(x)
	xnames <- names(x)
	xtypes <- vector("character", xncol)
	xlevels <- list()
	for(i in 1:xncol){
		if(is.numeric(x[,i])){
			xtypes[i] <- "numeric"
		}
		else if(is.factor(x[,i]) || is.character(x[,i]) || is.logical(x[,i])){
			xtypes[i] <- "factor"
			x[,i] <- as.factor(x[,i])
			xlevels[[xnames[i]]] <- levels(x[,i])
		}
		else
			stop("Conditional attributes can only be of numeric, factor, character or logical type")
	}
	list(xnames = xnames, xtypes = xtypes, xlevels = xlevels)
}

crules <- function(formula, data, q, qsplit = q, weights)
{
	params <- .prepare.data(formula, data, q, qsplit, weights)
	#create object and call the method
	rarc <- new(RInterface)
	
	rules <- rarc$generateRules(params)
	
	rm(rarc)
	#return object of crules class
	new("crules", rules = rules,  yname = params$yname,
			ylevels = params$ylevels, xnames = params$xnames,
			xlevels = params$xlevels, xtypes = params$xtypes, call = match.call(expand.dots = FALSE))
}

setMethod("summary", "crules", function(object){
			value <- vector("numeric", 5)
			value[1] <- length(object@rules$NumbersOfConditions)
			value[2] <- mean(object@rules$NumbersOfConditions)
			value[3] <- mean(object@rules$RulesPrecisions)
			value[4] <- mean(object@rules$RulesCoverages)
			value[5] <- mean(object@rules$Pvalues)
			data.frame(value, row.names = c("Number of rules:", 
							"Average number of elementary conditions per rule:",
							"Average precision of rules:",
							"Average coverage of rules:",
							"Average p-value of rules:"))
		})

.prep.pred.res <- function(preds, classes){
	preds$predictions <- factor(classes[preds$predictions + 1], levels = classes)
	
	if(length(preds$confusionMatrix) > 0)
	{
		cm.names <- list(Actual = classes,
				Predicted = classes)
		cm <- matrix(unlist(preds$confusionMatrix),
				ncol=length(preds$confusionMatrix[[1]]),
				byrow=TRUE, dimnames = cm.names)
		
		addInfo <- matrix(c(preds$unclassified, preds$classesAccuracies), 
				ncol = 2, dimnames = list(classes, 
						c("Unknown", "Class accuracy")))
		preds$confusionMatrix <- cbind(cm, addInfo)
		names(dimnames(preds$confusionMatrix)) <- c("Actual", "Predicted")
	}
	else
	{
		preds$confusionMatrix <- NULL
		preds$acc <- NULL
		preds$bac <- NULL
	}
	
	preds$classesAccuracies <- NULL
	preds$unclassified <- NULL
	preds
}

setMethod("predict", "crules", function(object, newdata, weights){
			yindex <- match(object@yname, names(newdata))
			if(is.na(yindex))
				y <- vector("numeric")
			else
				y <- factor(newdata[,yindex], levels = object@ylevels)
			
			#weights:
			weights <- .check.weights(weights, nrow(newdata))
			
			cols <- match(object@xnames, names(newdata))
			if(NA %in% cols)
				stop("Attributes in new data should match attributes used to generate rules")
			x <- newdata[,cols, drop = FALSE]
			xncol <- ncol(x)
			xnames <- names(x)
			xtypes <- object@xtypes
			xlevels <- object@xlevels
			for(i in 1:xncol){
				if(is.numeric(x[,i])){
					if(xtypes[i] != "numeric")
						stop("Wrong attribute type")
				}
				else if(is.factor(x[,i]) || is.character(x[,i]) || is.logical(x[,i])){
					if(xtypes[i] != "factor")
						stop("Wrong attribute type")
					x[,i] <- factor(x[,i], levels = xlevels[[xnames[[i]]]])
				}
				else
					stop("Conditional attributes can only be of numeric, factor, character or logical type")
			}
						
			params <- list(y = y, yname = object@yname, ylevels = object@ylevels, x = x, xtypes = xtypes, 
						   xnames = xnames, xlevels = xlevels, rules = object@rules$Rules, 
						   confidenceDegrees = object@rules$ConfidenceDegrees, weights = weights, seed = runif(1))
			rarc <- new(RInterface)
			preds <- rarc$predict(params)
			
			rm(rarc)
			.prep.pred.res(preds, object@ylevels)
		})
