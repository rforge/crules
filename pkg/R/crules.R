setClass("crules", representation(rules = "list",  yname = "character",
				ylevels = "character", xnames = "character",
				xlevels = "list", xtypes = "character",
				call = "call"))


crules.print <- function(x){
	print(data.frame(Rule = x@rules$Rules, Precision = x@rules$RulesPrecisions, 
					Coverage = x@rules$RulesCoverages, "P-value" = x@rules$Pvalues), 
			right = FALSE)
}

setMethod("print", "crules", crules.print)
#setMethod("show", "crules", crules.print)

q.names = c("g2", "lift", "ls", "rss", "corr", "s", "c1", "c2", "cn2", "gain")
qsplit.names <- c(q.names, "entropy")

prepare.data <- function(formula, data, q, qsplit, weights){
	#measures
	if(is.character(q)){
		
		q <- match.arg(q, q.names)
		qfun <- NULL
	}
	else if(is.function(q)){
		qfun <- q
		qsplit  #forcing the promise
		q <- ""
	}
	if(is.character(qsplit)){
		
		qsplit <- match.arg(qsplit, qsplit.names)
		qsplitfun <- NULL
	}
	else if(is.function(qsplit)){
		qsplitfun <- qsplit
		qsplit <- ""
	}
	
	#data evaluation
	mf <- match.call(expand.dots = FALSE)
	m <- match(c("formula", "data"), names(mf), 0)
	mf <- mf[c(1, m)]
	mf$na.action <- na.pass
	mf$drop.unused.levels <- FALSE
	mf[[1]] <- as.name("model.frame") 
	m <- eval.parent(mf)
	y <- model.response(m)
	x <- m[, -1, drop = FALSE]
	if(is.numeric(y)) stop("Response cannot be of numerical type")
	#weights:
	if(missing(weights))
		weights <- vector("numeric")
	else if(length(weights) != length(y))
		stop("weights vector should be the same length as the number of examples")
	else if(any(weights <= 0))
		stop("weights cannot be negative numbers")
	#levels, names, types
	yname <- names(m)[1]
	ylevels <- levels(m[,1])
	xncol <- ncol(x)
	xnames <- vector("character", xncol)
	xtypes <- vector("character", xncol)
	xlevels <- vector("list", xncol)
	for(i in 1:xncol){
		xnames[i] <- names(x)[i]
		xtypes[i] <- class(x[,i])
		if(xtypes[i] != "numeric" && xtypes[i] != "integer"){
			xlevels[[i]] <- levels(x[,i])
		}
	}
	list(y = y, yname = yname, ylevels = ylevels, x = x, xtypes = xtypes, xnames = xnames,
			xlevels = xlevels, q = q, qsplit = qsplit, qfun = qfun, qsplitfun = qsplitfun,
			weights = weights)
}

crules <- function(formula, data, q, qsplit = q, weights)
{
	par <- prepare.data(formula, data, q, qsplit, weights)
	#create object and call the method
	rarc <- new( RInterface)
	rules <- rarc$generateRules( par$y, par$yname, par$ylevels,  par$x, par$xtypes, 
			par$xnames, par$xlevels, 
			par$q, par$qsplit, par$qfun, par$qsplitfun, 
			par$weights )
	#return object of crules class
	new("crules", rules = rules,  yname = par$yname,
			ylevels = par$ylevels, xnames = par$xnames,
			xlevels = par$xlevels, xtypes = par$xtypes, call = match.call(expand.dots = FALSE))
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

prep.pred.res <- function(preds, classes){
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
				y <- newdata[,yindex]
			
			#weights:
			if(missing(weights))
				weights <- vector("numeric")
			else if(length(weights) != length(y))
				stop("weights vector should be the same length as the number of examples")
			else if(any(weights <= 0))
				stop("weights cannot be negative numbers")
			
			cols <- match(object@xnames, names(newdata))
			x <- newdata[cols]
			xncol <- ncol(x)
			xnames <- vector("character", xncol)
			xtypes <- vector("character", xncol)
			xlevels <- vector("list", xncol)
			for(i in 1:xncol){
				xnames[i] <- names(x)[i]
				xtypes[i] <- class(x[,i])
				if(xtypes[i] != "numeric" && xtypes[i] != "integer"){
					xlevels[[i]] <- levels(x[,i])
				}
			}
			if(length(xnames) < length(object@xnames))
				stop("Attributes in new data should match attributes used to generate rules")
			rarc <- new( RInterface)
			preds <- rarc$predict(y, object@yname, object@ylevels, x, xtypes, xnames, xlevels,
					object@rules$Rules, object@rules$ConfidenceDegrees, weights)
			
			
			prep.pred.res(preds, object@ylevels)
		})
