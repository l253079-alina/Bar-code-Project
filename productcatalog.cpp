#include "productcatalog.h"

static const int barcodeLength = 6;
static const int categoryIdLength = 2;
static const int barcodeSuffixLength = 4;
static const int highestBarcodeSuffix = 9999;
static const int defaultProductIdWidth = 4;

ProductCatalog::ProductCatalog(ProductRepository& repositoryToUse)
    : productRepository(repositoryToUse)
{
}

bool ProductCatalog::reload(QString& problemDescription){
    return productRepository.loadProducts(products, problemDescription);
}

optional<Product> ProductCatalog::findByBarcode(const QString& barcode) const{
    const QString trimmedBarcode = barcode.trimmed();
    for (const Product& product : products) {
        if (product.barcode == trimmedBarcode)
            return product;
    }
    return nullopt;
}

optional<Product> ProductCatalog::findByProductId(const QString& productId) const{
    for (const Product& product : products) {
        if (product.productId == productId)
            return product;
    }
    return nullopt;
}

optional<Product> ProductCatalog::findByAcronym(const QString& acronym) const{
    for (const Product& product : products) {
        if (product.acronym.compare(acronym, Qt::CaseInsensitive) == 0)
            return product;
    }
    return nullopt;
}

const vector<Product>& ProductCatalog::allProducts() const{
    return products;
}

bool ProductCatalog::acronymInUse(const QString& acronym, const QString& exceptProductId) const{
    for (const Product& product : products) {
        if (product.productId == exceptProductId)
            continue;
        if (product.acronym.compare(acronym, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

bool ProductCatalog::categoryInUse(const QString& categoryId) const{
    for (const Product& product : products) {
        if (product.categoryId == categoryId)
            return true;
    }
    return false;
}

QString ProductCatalog::createNextProductId() const{
    long long highestNumber = 0;
    int widestIdLength = 0;
    for (const Product& product : products) {
        bool idIsNumber = false;
        const long long idNumber = product.productId.toLongLong(&idIsNumber);
        if (!idIsNumber)
            continue;
        highestNumber = max(highestNumber, idNumber);
        widestIdLength = max(widestIdLength, static_cast<int>(product.productId.size()));
    }
    if (widestIdLength == 0)
        widestIdLength = defaultProductIdWidth;
    return QString("%1").arg(highestNumber + 1, widestIdLength, 10, QChar('0'));
}

bool ProductCatalog::nextBarcodeForCategory(const QString& categoryId, QString& barcode, QString& problemDescription) const{
    int highestSuffixInUse = 0;
    for (const Product& product : products) {
        if (product.barcode.size() != barcodeLength || !product.barcode.startsWith(categoryId))
            continue;
        bool suffixIsNumber = false;
        const int suffixNumber = product.barcode.mid(categoryIdLength).toInt(&suffixIsNumber);
        if (suffixIsNumber)
            highestSuffixInUse = max(highestSuffixInUse, suffixNumber);
    }

    const int nextSuffix = highestSuffixInUse + 1;
    if (nextSuffix > highestBarcodeSuffix) {
        problemDescription = "Category " + categoryId + " has no free barcode numbers left.";
        return false;
    }
    barcode = categoryId + QString("%1").arg(nextSuffix, barcodeSuffixLength, 10, QChar('0'));
    return true;
}

bool ProductCatalog::saveChangedProducts(const vector<Product>& changedProducts, QString& problemDescription){
    if (!productRepository.saveProducts(changedProducts, problemDescription))
        return false;
    products = changedProducts;
    return true;
}

bool ProductCatalog::addProduct(const Product& newProduct, QString& problemDescription){
    if (findByProductId(newProduct.productId).has_value()) {
        problemDescription = "A product with id " + newProduct.productId + " already exists.";
        return false;
    }
    if (findByBarcode(newProduct.barcode).has_value()) {
        problemDescription = "A product with barcode " + newProduct.barcode + " already exists.";
        return false;
    }

    vector<Product> changedProducts = products;
    changedProducts.push_back(newProduct);
    return saveChangedProducts(changedProducts, problemDescription);
}

bool ProductCatalog::updateProduct(const Product& changedProduct, QString& problemDescription){
    vector<Product> changedProducts = products;
    bool productWasFound = false;
    for (Product& product : changedProducts) {
        if (product.productId == changedProduct.productId) {
            product = changedProduct;
            productWasFound = true;
        }
    }
    if (!productWasFound) {
        problemDescription = "Product " + changedProduct.productId + " was not found.";
        return false;
    }
    return saveChangedProducts(changedProducts, problemDescription);
}

bool ProductCatalog::setAvailableQuantity(const QString& productId, double newQuantity, QString& problemDescription){
    if (newQuantity < 0.0) {
        problemDescription = "The available quantity cannot be negative.";
        return false;
    }

    vector<Product> changedProducts = products;
    bool productWasFound = false;
    for (Product& product : changedProducts) {
        if (product.productId == productId) {
            product.availableQuantity = newQuantity;
            productWasFound = true;
        }
    }
    if (!productWasFound) {
        problemDescription = "Product " + productId + " was not found.";
        return false;
    }
    return saveChangedProducts(changedProducts, problemDescription);
}

bool ProductCatalog::adjustStock(const map<QString, double>& quantityToDeductByProductId, QString& problemDescription){
    if (quantityToDeductByProductId.empty())
        return true;

    vector<Product> changedProducts = products;
    for (Product& product : changedProducts) {
        const auto foundQuantity = quantityToDeductByProductId.find(product.productId);
        if (foundQuantity == quantityToDeductByProductId.end())
            continue;
        product.availableQuantity -= foundQuantity->second;
        if (product.availableQuantity < 0.0)
            product.availableQuantity = 0.0;
    }
    return saveChangedProducts(changedProducts, problemDescription);
}