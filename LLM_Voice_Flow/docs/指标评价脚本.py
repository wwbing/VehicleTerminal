"""
智能座舱意图识别与指令转换评估指标
专门针对结构化输出任务的评估方法
"""

def intent_accuracy(predictions, labels):
    """计算意图分类准确率"""
    correct = 0
    total = len(predictions)
    
    for pred, label in zip(predictions, labels):
        try:
            pred_intent = pred.split('###')[0].strip()
            label_intent = label.split('###')[0].strip()
            if pred_intent == label_intent:
                correct += 1
        except:
            # 格式错误的预测视为错误
            pass
    
    return correct / total * 100


def exact_match_accuracy(predictions, labels):
    """计算完全匹配准确率"""
    correct = 0
    total = len(predictions)
    
    for pred, label in zip(predictions, labels):
        if pred.strip() == label.strip():
            correct += 1
    
    return correct / total * 100


def command_similarity_accuracy(predictions, labels, threshold=0.6):
    """计算指令相似度准确率"""

    def calculate_similarity(str1, str2):
        """计算两个字符串的相似度"""
        # 使用字符级别的Jaccard相似度
        set1 = set(str1)
        set2 = set(str2)

        if len(set1) == 0 and len(set2) == 0:
            return 1.0

        intersection = len(set1.intersection(set2))
        union = len(set1.union(set2))

        return intersection / union if union > 0 else 0.0

    similarities = []
    valid_pairs = 0

    for pred, label in zip(predictions, labels):
        try:
            pred_parts = pred.split('###')
            label_parts = label.split('###')

            if len(pred_parts) == 2 and len(label_parts) == 2:
                pred_intent, pred_command = pred_parts[0].strip(), pred_parts[1].strip()
                label_intent, label_command = label_parts[0].strip(), label_parts[1].strip()

                # 只在意图正确的情况下评估指令相似度
                if pred_intent == label_intent:
                    valid_pairs += 1
                    similarity = calculate_similarity(pred_command, label_command)
                    similarities.append(similarity)
        except:
            pass

    if not similarities:
        return 0.0, 0.0

    # 计算平均相似度
    avg_similarity = sum(similarities) / len(similarities)

    # 计算超过阈值的比例
    above_threshold = sum(1 for sim in similarities if sim >= threshold) / len(similarities)

    return avg_similarity * 100, above_threshold * 100


def command_exact_accuracy(predictions, labels):
    """计算指令完全匹配准确率（作为对比）"""
    correct = 0
    valid_pairs = 0

    for pred, label in zip(predictions, labels):
        try:
            pred_parts = pred.split('###')
            label_parts = label.split('###')

            if len(pred_parts) == 2 and len(label_parts) == 2:
                pred_intent, pred_command = pred_parts[0].strip(), pred_parts[1].strip()
                label_intent, label_command = label_parts[0].strip(), label_parts[1].strip()

                # 只在意图正确的情况下评估指令
                if pred_intent == label_intent:
                    valid_pairs += 1
                    if pred_command == label_command:
                        correct += 1
        except:
            pass

    return correct / valid_pairs * 100 if valid_pairs > 0 else 0


def format_accuracy(predictions):
    """计算格式正确率"""
    correct_format = 0
    total = len(predictions)
    
    valid_intents = ['温湿度显示', '时间显示', '音乐播放', '音量控制', 
                    '地图导航', '倒车监控', '天气查询', '通用指令', '拒识']
    
    for pred in predictions:
        try:
            parts = pred.split('###')
            if len(parts) == 2:
                intent = parts[0].strip()
                command = parts[1].strip()
                
                # 检查意图是否在有效集合中，指令是否非空
                if intent in valid_intents and len(command) > 0:
                    correct_format += 1
        except:
            pass
    
    return correct_format / total * 100


def evaluate_intent_command_model(predictions, labels, similarity_threshold=0.6):
    """综合评估意图识别和指令转换模型"""
    results = {}

    # 1. 意图分类准确率（最重要）
    results['intent_accuracy'] = intent_accuracy(predictions, labels)

    # 2. 指令相似度准确率
    avg_similarity, above_threshold = command_similarity_accuracy(predictions, labels, similarity_threshold)
    results['command_avg_similarity'] = avg_similarity
    results['command_similarity_accuracy'] = above_threshold

    # 3. 指令完全匹配准确率（作为对比）
    results['command_exact_accuracy'] = command_exact_accuracy(predictions, labels)

    # 4. 格式正确率
    results['format_accuracy'] = format_accuracy(predictions)

    # 5. 分层统计
    intent_correct = 0
    format_correct = 0
    total = len(predictions)

    valid_intents = ['温湿度显示', '时间显示', '音乐播放', '音量控制',
                    '地图导航', '倒车监控', '天气查询', '通用指令', '拒识']

    # 指令相似度统计
    similarities = []
    command_pairs = []

    def calculate_similarity(str1, str2):
        set1 = set(str1)
        set2 = set(str2)
        if len(set1) == 0 and len(set2) == 0:
            return 1.0
        intersection = len(set1.intersection(set2))
        union = len(set1.union(set2))
        return intersection / union if union > 0 else 0.0

    for pred, label in zip(predictions, labels):
        try:
            pred_parts = pred.split('###')
            label_parts = label.split('###')

            if len(pred_parts) == 2 and len(label_parts) == 2:
                pred_intent, pred_command = pred_parts[0].strip(), pred_parts[1].strip()
                label_intent, label_command = label_parts[0].strip(), label_parts[1].strip()

                # 格式检查
                if pred_intent in valid_intents and len(pred_command) > 0:
                    format_correct += 1

                # 意图检查
                if pred_intent == label_intent:
                    intent_correct += 1

                    # 计算指令相似度
                    similarity = calculate_similarity(pred_command, label_command)
                    similarities.append(similarity)
                    command_pairs.append((pred_command, label_command, similarity))
        except:
            pass

    results['detailed_stats'] = {
        'intent_correct': intent_correct,
        'format_correct': format_correct,
        'total': total,
        'command_similarities': similarities,
        'command_pairs': command_pairs
    }

    return results


def calculate_weighted_score(results, weights=None):
    """计算加权综合得分"""
    if weights is None:
        # 调整后的权重：更重视意图和指令相似度
        weights = {
            'intent_accuracy': 0.5,
            'command_similarity_accuracy': 0.3,
            'format_accuracy': 0.2
        }

    score = 0
    for metric, weight in weights.items():
        if metric in results:
            score += results[metric] * weight

    return score


if __name__ == "__main__":
    # 测试样例
    test_cases = [
        {
            "prediction": "温湿度显示###显示车内温度",
            "label": "温湿度显示###显示车内温度"
        },
        {
            "prediction": "时间显示###显示当前时间",
            "label": "时间显示###显示时间"
        },
        {
            "prediction": "音乐播放###播放音乐",
            "label": "音乐播放###播放音乐"
        },
        {
            "prediction": "地图导航###打开导航功能",
            "label": "地图导航###显示导航界面"
        },
        {
            "prediction": "天气查询###查询今天天气",
            "label": "天气查询###查询今天天气"
        },
        {
            "prediction": "格式错误的输出",
            "label": "拒识###无法识别指令"
        }
    ]
    
    predictions = [case["prediction"] for case in test_cases]
    labels = [case["label"] for case in test_cases]
    
    # 执行评估
    results = evaluate_intent_command_model(predictions, labels, similarity_threshold=0.6)

    print("=== 智能座舱意图识别评估结果 ===")
    print(f"意图分类准确率: {results['intent_accuracy']:.2f}%")
    print(f"指令平均相似度: {results['command_avg_similarity']:.2f}%")
    print(f"指令相似度准确率(>60%): {results['command_similarity_accuracy']:.2f}%")
    print(f"指令完全匹配准确率: {results['command_exact_accuracy']:.2f}%")
    print(f"格式正确率: {results['format_accuracy']:.2f}%")

    print("\n=== 详细统计 ===")
    stats = results['detailed_stats']
    print(f"总样本数: {stats['total']}")
    print(f"意图正确数: {stats['intent_correct']}")
    print(f"格式正确数: {stats['format_correct']}")

    # 计算加权得分
    weighted_score = calculate_weighted_score(results)
    print(f"\n=== 加权综合得分 ===")
    print(f"综合得分: {weighted_score:.2f}%")
    print("权重分配: 意图准确率(50%) + 指令相似度准确率(30%) + 格式正确率(20%)")

    
